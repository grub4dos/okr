

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "volume.h"
#include "utils.h"
#include "types.h"
#include "dir.h"
#include "ntfstime.h"

#include "ntfs.h"

//EFI_DISK_PARAMETER *g_efidisk=NULL;
ntfs_volume *g_vol=NULL;

DWORD n_check_vol_readonly(void *vol)
{
	return (DWORD)NVolReadOnly(((ntfs_volume*)vol));
}

DWORD n_init(EFI_DISK_PARAMETER *efidisk, DWORD bReadOnly)
{
	unsigned long flag=0;

	if(g_vol)
	{
		n_uninit(TRUE);
	}

	if(bReadOnly)
	{
		flag |= NTFS_MNT_RDONLY;
	}

	g_vol=utils_mount_volume(efidisk, flag);

	if(g_vol==NULL)
	{
		return N_FAIL;
	}

	if (ntfs_volume_get_free_space(g_vol)) {
		ntfs_log_perror("ERROR: couldn't get free space");
		ntfs_umount(g_vol, FALSE);
		g_vol=NULL;
		return N_FAIL;
	}

//	g_efidisk=efidisk;

	return N_SUCCESS;
}

DWORD n_uninit(DWORD force)
{
	if(g_vol)
	{
		ntfs_umount(g_vol, (BOOL)force);
		g_vol=NULL;
	}

	return N_SUCCESS;
}

ntfs_inode * create_dir(ntfs_inode *parent_ni,char* dir)
{
	ntfschar *ufilename;
	/* inode to the file that is being created */
	ntfs_inode *ni=NULL;
	int ufilename_len;

	/* ntfs_mbstoucs(...) will allocate memory for ufilename if it's NULL */
	ufilename = NULL;
	ufilename_len = ntfs_mbstoucs(dir, &ufilename);
	if (ufilename_len == -1) {
		ntfs_log_perror("ERROR: Failed to convert '%s' to unicode",
			dir);
		return NULL;
	}
	ni = ntfs_create(parent_ni, 0, ufilename, ufilename_len, S_IFDIR);
	free(ufilename);

	return ni;
}

//
// 新建目录，目录必须不存在，否则返回错误N_FILE_EXISTS
//           父目录必须存在，否则返回错误N_PATH_NOT_FOUND
// char* path - 全路径 \dir\file，最后字符不能为路径分符'\'
//
DWORD n_createdir(char* path)
{
	char *lastp;
	ntfs_inode *parent_ni=NULL;
	ntfs_inode *ni=NULL;
	DWORD ret=N_SUCCESS;

	if ((path == NULL))
		return N_INVALID_PARAMETER;

	//至少要2个字符以上 "\d"
	if(strlen(path)<2)
	{
		ntfs_log_perror("strlen(path)<2. %s\n",path);
		return N_INVALID_PARAMETER;
	}

	ni = ntfs_pathname_to_inode(g_vol, NULL, path);
	if(ni)
	{
		ntfs_inode_close(ni);
		ntfs_log_perror("dir exists. %s\n",path);
		return N_FILE_EXISTS;
	}

	//查找父目录
	lastp = path + strlen(path) - 1;
	while(lastp!= path && !ISPATHSEPARATOR(*lastp))
	{
		lastp--;
	}

	if(lastp==path)
	{
		//根目录
		parent_ni= ntfs_inode_open(g_vol, FILE_root);
	}
	else
	{
		*lastp='\0';

		parent_ni= ntfs_pathname_to_inode(g_vol, NULL, path);

		*lastp=PATH_SEP;
	}

	if(parent_ni==NULL)
	{
		ntfs_log_perror("parent_ni==NULL. %s\n",path);
		return N_PATH_NOT_FOUND;
	}

	ni = create_dir(parent_ni,lastp+1);
	if(ni)
	{
		ntfs_inode_close(ni);
	}
	else
	{
		ret = N_FAIL;
		ntfs_log_perror("create dir failed.ni==NULL. %s\n",path);
	}

	if(parent_ni)
	{
		ntfs_inode_close(parent_ni);
	}

	return ret;
}

//
// 删除目录，不存在，则返回N_PATH_NOT_FOUND
//           
// char* org_path - 全路径 /dir/subdir
//
DWORD n_deletedir(char* org_path)
{
	return n_deletefile(org_path);
}

typedef struct {
	ntfs_volume *vol;
	long long list_buf_size;
	PDIR_LIST	pdirlist;
	DWORD	ret;
} ntfsls_dirent;

/**
 * list_dir_entry
 *
 */
static int list_dir_entry(ntfsls_dirent * dirent, const ntfschar * name,
			  const int name_len, const int name_type,
			  const s64 pos ,
			  const MFT_REF mref, const unsigned dt_type)
{
	char *filename = NULL;
	int result = 0;

	ntfs_log_trace("Entering...result=%lu,pos=%lld,mref=%lld,dirent->pdirlist->count=%lld\n",result,pos,mref,dirent->pdirlist->count);

	filename = calloc(1, MAX_PATH);
	if (!filename)
	{
		ntfs_log_error("calloc(1, MAX_PATH)==null.mref=%lld,pos=%lld\n",mref,pos);
		return -1;
	}

	if (ntfs_ucstombs(name, name_len, &filename, MAX_PATH) < 0) {
		ntfs_log_error("Cannot represent filename in current locale.\n");
		result=-1;
		goto free;
	}

	result = 0;					// These are successful
// 	if ((MREF(mref) < FILE_first_user) && (!opts.system))
// 		goto free;
// 	if (name_type == FILE_NAME_POSIX && !opts.all)
// 		goto free;
// 	if (((name_type & FILE_NAME_WIN32_AND_DOS) == FILE_NAME_WIN32) )
// 		goto free;
	if (((name_type & FILE_NAME_WIN32_AND_DOS) == FILE_NAME_DOS) )
		goto free;

// 	if (dt_type == NTFS_DT_DIR && opts.classify)
// 		sprintf(filename + strlen(filename), "/");

	ntfs_log_trace("pos=%lld,'%s'.\n",pos,filename);

	if (dt_type == NTFS_DT_DIR)
	{
	    if( strcmp(filename, ".")==0 
	    || strcmp(filename, "..")==0 
		)
		{
			goto free;
		}
	}

	if(dirent->ret==N_NOT_ENOUGH_MEMORY
		||((s64)(dirent->pdirlist->count*sizeof(DIR_ENTRY)+sizeof(DIR_LIST))>dirent->list_buf_size))
	{
		ntfs_log_trace("N_NOT_ENOUGH_MEMORY,dirent->list_buf_size=%lld,dirent->pdirlist->count=%lld,dirent->pdirlist->count*sizeof(DIR_ENTRY)+sizeof(DIR_LIST)=%llu\n",
			dirent->list_buf_size,dirent->pdirlist->count,dirent->pdirlist->count*sizeof(DIR_ENTRY)+sizeof(DIR_LIST));
		dirent->pdirlist->count++;
		goto free;
	}
	else
	{
		s64 filesize = 0;
		ntfs_inode *ni;
		ntfs_attr_search_ctx *ctx = NULL;
		FILE_NAME_ATTR *file_name_attr;
		ATTR_RECORD *attr;
		struct timespec change_time;
 		char t_buf[26];

		result = -1;				// Everything else is bad

		ni = ntfs_inode_open(dirent->vol, mref);
		if (!ni)
			goto release;

		ctx = ntfs_attr_get_search_ctx(ni, NULL);
		if (!ctx)
			goto release;

		if (ntfs_attr_lookup(AT_FILE_NAME, AT_UNNAMED, 0, 0, 0, NULL,
				0, ctx))
			goto release;
		attr = ctx->attr;

		file_name_attr = (FILE_NAME_ATTR *)((char *)attr +
				le16_to_cpu(attr->value_offset));
		if (!file_name_attr)
			goto release;

		change_time = ntfs2timespec(file_name_attr->last_data_change_time);
		strcpy(t_buf, ctime(&change_time.tv_sec));
		memmove(t_buf+16, t_buf+19, 5);
		t_buf[21] = '\0';

		dirent->pdirlist->entry[dirent->pdirlist->count].filesize=0;
		if (dt_type != NTFS_DT_DIR) {
			if (!ntfs_attr_lookup(AT_DATA, AT_UNNAMED, 0, 0, 0,
					NULL, 0, ctx))
			{
				filesize = ntfs_get_attribute_value_length(
						ctx->attr);
				dirent->pdirlist->entry[dirent->pdirlist->count].filesize=filesize;
			}
			dirent->pdirlist->entry[dirent->pdirlist->count].type=N_TYPE_FILE;
		}
		else
		{
			dirent->pdirlist->entry[dirent->pdirlist->count].type=N_TYPE_DIR;
		}
		
		ntfs_log_trace("%8lld %s %s\n", (long long)filesize, t_buf + 4,
 				filename);

		dirent->pdirlist->entry[dirent->pdirlist->count].modifytime=change_time.tv_sec;
		dirent->pdirlist->entry[dirent->pdirlist->count].namelen=name_len;
		strncpy(dirent->pdirlist->entry[dirent->pdirlist->count].name,filename,MAX_PATH);

		dirent->pdirlist->count++;

		result = 0;
release:
		/* Release attribute search context and close the inode. */
		if (ctx)
			ntfs_attr_put_search_ctx(ctx);
		if (ni)
			ntfs_inode_close(ni);
	}

free:
	free(filename);

	ntfs_log_trace("Exit...result=%lu,pos=%lld,mref=%lld,dirent->pdirlist->count=%lld\n",result,pos,mref,dirent->pdirlist->count);

	return result;
}

//
// 列目录
// char* path - 全路径 /dir/subdir
// long long *psize - buf大小
// void* buf - 如果buf为NULL或buf size不足，则在psize中返回buf所需的大小，并返回N_NOT_ENOUGH_MEMORY
// 
DWORD n_listdir(char* path,long long *psize,void* buf)
{
	DWORD ret=N_SUCCESS;
	int result;
	ntfs_inode *ni=NULL;
	s64 pos=0;
	ntfsls_dirent dirent;
	DIR_LIST dirlist;

	//ntfs_log_trace("Entering...\n");

	ni = ntfs_pathname_to_inode(g_vol, NULL, path);
	if (!ni) {
		ntfs_log_error("ntfs_pathname_to_inode=null.%s\n",path);
		return N_PATH_NOT_FOUND;
	}

	dirent.vol=g_vol;
	if(buf==NULL||*psize<sizeof(DIR_LIST))
	{
		ntfs_log_trace("buf==NULL||*psize<sizeof(DIR_LIST),buf=%p,*psize=%lld\n",buf,*psize);
		ret=N_NOT_ENOUGH_MEMORY;
		dirent.pdirlist=&dirlist;
		dirent.list_buf_size=sizeof(DIR_LIST);
	}
	else
	{
		dirent.list_buf_size=*psize;
		dirent.pdirlist=(PDIR_LIST)buf;
	}
	dirent.pdirlist->count=0;
	dirent.ret=ret;

	result = ntfs_readdir(ni, &pos, &dirent, (ntfs_filldir_t) list_dir_entry);

	if (result == 0) 
	{
	}
	else
	{
		switch(errno)
		{
		case EINVAL:
			ret=N_INVALID_PARAMETER;
			break;
		case ENOTDIR:
			ret=N_NOT_DIRECTORY;
			break;
		default:
			ret=N_FAIL;
			break;
		}
		ntfs_log_error("ntfs_readdir failed.%s,errno=%d,ret=%x\n",path);
	}

	if(ni)
	{
		ntfs_inode_close(ni);
	}

	//ntfs_log_trace("Exit...\n");

	return ret;
}

//
// 打开文件，文件必须要存在，否则返回错误N_PATH_NOT_FOUND
// char* path - 全路径 \dir\file
// NFILE* file - 如果成功，file返回文件句柄，失败则NULL
//
DWORD n_openfile(char* path,NFILE* file)
{
	ntfs_inode *ni=NULL;
	DWORD ret=N_SUCCESS;

	if ((path == NULL))
		return N_INVALID_PARAMETER;

	//至少要2个字符以上 "\d"
	if(strlen(path)<2)
	{
		ntfs_log_perror("strlen(path)<2. %s\n",path);
		return N_INVALID_PARAMETER;
	}

	ni= ntfs_pathname_to_inode(g_vol, NULL, path);
	if(!ni)
	{
		ntfs_log_perror("file not found. %s\n",path);
		return N_FILE_NOT_FOUND;
	}

	if(file)
	{
		*file=(NFILE)ni;
	}

	return ret;
}

static ntfs_inode * create_file(ntfs_inode *parent_ni,char* file)
{
	ntfschar *ufilename;
	/* inode to the file that is being created */
	ntfs_inode *ni=NULL;
	int ufilename_len;

	/* ntfs_mbstoucs(...) will allocate memory for ufilename if it's NULL */
	ufilename = NULL;
	ufilename_len = ntfs_mbstoucs(file, &ufilename);
	if (ufilename_len == -1) {
		ntfs_log_perror("ERROR: Failed to convert '%s' to unicode",
			file);
		return NULL;
	}
	ni = ntfs_create(parent_ni, 0, ufilename, ufilename_len, S_IFREG);
	free(ufilename);

	return ni;
}

//
// 新建文件，文件必须不存在，否则返回错误N_FILE_EXISTS
//           父目录必须存在，否则返回错误N_PATH_NOT_FOUND
// char* path - 全路径 \dir\file
// NFILE* file - 如果成功，file返回文件句柄，失败则NULL
//
DWORD n_createfile(char* path,NFILE* file)
{
	char *lastp;
	ntfs_inode *parent_ni=NULL;
	ntfs_inode *ni=NULL;
	DWORD ret=N_SUCCESS;

	if ((path == NULL))
		return N_INVALID_PARAMETER;

	//至少要2个字符以上 "\d"
	if(strlen(path)<2)
	{
		ntfs_log_perror("strlen(path)<2. %s\n",path);
		return N_INVALID_PARAMETER;
	}

	ni= ntfs_pathname_to_inode(g_vol, NULL, path);
	if(ni)
	{
		ntfs_inode_close(ni);
		ntfs_log_perror("file exists. %s\n",path);
		return N_FILE_EXISTS;
	}

	//查找父目录
	lastp = path + strlen(path) - 1;
	while(lastp!= path && !ISPATHSEPARATOR(*lastp))
	{
		lastp--;
	}

	if(lastp==path)
	{
		//根目录
		parent_ni= ntfs_inode_open(g_vol, FILE_root);
	}
	else
	{
		*lastp='\0';

		parent_ni= ntfs_pathname_to_inode(g_vol, NULL, path);

		*lastp=PATH_SEP;
	}

	if(parent_ni==NULL)
	{
		ntfs_log_perror("parent_ni==NULL. %s\n",path);
		return N_PATH_NOT_FOUND;
	}

	ni = create_file(parent_ni,lastp+1);
	if(ni)
	{
		if(file)
		{
			*file=(NFILE)ni;
		}
		else
		{
			ntfs_inode_close(ni);
		}
	}
	else
	{
		ret = N_FAIL;
		ntfs_log_perror("create file failed.ni==NULL. %s\n",path);
	}

	if(parent_ni)
	{
		ntfs_inode_close(parent_ni);
	}

	return ret;
}

//
// 删除文件，不存在，则返回N_PATH_NOT_FOUND
//           
// char* path - 全路径 \dir\file
//
DWORD n_deletefile(char* org_path)
{
	char *name=NULL;
	ntfschar *uname = NULL;
	ntfs_inode *dir_ni = NULL, *ni;
	char *path=NULL;
	int uname_len;
	DWORD ret=N_SUCCESS;

	path = strdup(org_path);
	if (!path)
	{
		return N_NOT_ENOUGH_MEMORY;
	}

	/* Open object for delete. */
	ni = ntfs_pathname_to_inode(g_vol, NULL, path);
	if (!ni) {
		ret = N_PATH_NOT_FOUND;
		goto exit;
	}

	/* deny unlinking metadata files */
	if (ni->mft_no < FILE_first_user) {
		ret=N_METAFILE;
		goto exit;
	}

	/* Generate unicode filename. */
	name = strrchr(path, PATH_SEP);
	name++;
	uname_len = ntfs_mbstoucs(name, &uname);
	if (uname_len < 0) {
		ret=N_NOT_ENOUGH_MEMORY;
		goto exit;
	}
	/* Open parent directory. */
	*--name = 0;
	dir_ni = ntfs_pathname_to_inode(g_vol, NULL, path);
	if (!dir_ni) {
		ret = N_PATH_NOT_FOUND;
		goto exit;
	}

	if (ntfs_delete(g_vol, org_path, ni, dir_ni,
		uname, uname_len))
		ret = -errno;
	/* ntfs_delete() always closes ni and dir_ni */
	ni = dir_ni = NULL;

exit:
	if(dir_ni)
		ntfs_inode_close(dir_ni);
	if(ni)
		ntfs_inode_close(ni);
	if(uname)
		free(uname);
	if(path)
		free(path);

	return N_SUCCESS;
}

//
// 关闭文件
// NFILE* file - 文件句柄
//
DWORD n_closefile(NFILE file)
{
	while (ntfs_inode_close((ntfs_inode *)file)) {
		if (errno != EBUSY) {
			ntfs_log_error("Sync failed. Run chkdsk.\n");
			break;
		}
		ntfs_log_error("Device busy.  Will retry sync in 3 seconds.\n");
		sleep(3);
	}

	return N_SUCCESS;
}

// 读取文件
// NFILE file - 文件句柄
// long long offset - 文件偏移
// long long size - 读取的内容大小，byte
// void* buf - 读入内容的缓存地址
// long long *readsize - 实际读取的大小
//
DWORD n_readfile(NFILE file,long long offset,long long size,void* buf,long long *readsize)
{
	DWORD ret=N_FAIL;
	ntfs_inode *out=(ntfs_inode *)file;
	ntfs_attr *na;
	ntfschar *attr_name;
	int attr_name_len = 0;
	ATTR_TYPES	 attribute=AT_DATA;
	s64 br;

	if(readsize)
		*readsize = 0;

	attr_name = ntfs_str2ucs(NULL, &attr_name_len);
	if (!attr_name) {
		ntfs_log_perror("ERROR: Failed to parse attribute name = NULL"
			);
		goto end;
	}

	na = ntfs_attr_open(out, attribute, attr_name, attr_name_len);
	if (!na) {
		ntfs_log_perror("ERROR: Couldn't open attribute");
		goto end;
	}
	ntfs_ucsfree(attr_name);

	ntfs_log_verbose("file size: %lld\n", (long long)na->data_size);
	if (size+offset>na->data_size) {
		ntfs_log_warning("size+offset>na->data_size,offset=%lld,size=%lld,na->data_size=%lld\n",offset,size,na->data_size);
		size=na->data_size-offset;
		ntfs_log_warning("size+offset>na->data_size,set new size,offset=%lld,size=%lld,na->data_size=%lld\n",offset,size,na->data_size);
	}

	ntfs_log_verbose("Starting read.\n");

	br = ntfs_attr_pread(na, offset, size, buf);
	if (br != size) {
		ntfs_log_warning("ntfs_attr_pread br != size,br=%lld,size=%lld\n",br,size);
	}

	if ((na->data_flags & ATTR_COMPRESSION_MASK)
		&& ntfs_attr_pclose(na))
		ntfs_log_perror("ERROR: ntfs_attr_pclose failed\n");

	if(readsize)
		*readsize = br;

	ret = N_SUCCESS;
// close_attr:
	ntfs_attr_close(na);

end:

	return ret;
}

// 写入文件
// NFILE file - 文件句柄
// long long offset - 文件偏移
// long long size - 写入的内容大小，byte
// void* buf - 写入内容的缓存地址
// long long *writesize - 实际写入的大小
//
DWORD n_writefile(NFILE file,long long offset,long long size,void* buf,long long *writesize)
{
	DWORD ret=N_FAIL;
	ntfs_inode *out=(ntfs_inode *)file;
	ntfs_attr *na;
	ntfschar *attr_name;
	int attr_name_len = 0;
	ATTR_TYPES	 attribute=AT_DATA;
	s64 bw;

	if(writesize)
		*writesize = 0;

	attr_name = ntfs_str2ucs(NULL, &attr_name_len);
	if (!attr_name) {
		ntfs_log_perror("ERROR: Failed to parse attribute name = NULL"
			);
		goto end;
	}

	na = ntfs_attr_open(out, attribute, attr_name, attr_name_len);
	if (!na) {
		if (errno != ENOENT) {
			ntfs_log_perror("ERROR: Couldn't open attribute");
			goto end;
		}
		/* Requested attribute isn't present, add it. */
		if (ntfs_attr_add(out, attribute, attr_name,
			attr_name_len, NULL, 0)) {
				ntfs_log_perror("ERROR: Couldn't add attribute");
				goto end;
		}
		na = ntfs_attr_open(out, attribute, attr_name,
			attr_name_len);
		if (!na) {
			ntfs_log_perror("ERROR: Couldn't open just added "
				"attribute");
			goto end;
		}
	}
	ntfs_ucsfree(attr_name);

	ntfs_log_verbose("Old file size: %lld\n", (long long)na->data_size);
	if (na->data_size < (offset+size)) {
		if (ntfs_attr_truncate_solid(na, offset+size)) {
			ntfs_log_perror("ERROR: Couldn't resize attribute.newsize=%lld\n",offset+size);
			goto close_attr;
		}
	}

	ntfs_log_verbose("Starting write.\n");

	bw = ntfs_attr_pwrite(na, offset, size, buf);
	if (bw != size) {
		ntfs_log_perror("ERROR: ntfs_attr_pwrite failed\n");
		//			break;
	}

	if ((na->data_flags & ATTR_COMPRESSION_MASK)
		&& ntfs_attr_pclose(na))
		ntfs_log_perror("ERROR: ntfs_attr_pclose failed\n");

	if(writesize)
		*writesize = bw;

	ret = N_SUCCESS;
close_attr:
	ntfs_attr_close(na);

end:

	return ret;
}

// 设置文件大小
// NFILE file - 文件句柄
// long long size - 写入的内容大小，byte
//
DWORD n_setfilesize(NFILE file,long long size)
{
	DWORD ret=N_FAIL;
	ntfs_inode *out=(ntfs_inode *)file;
	ntfs_attr *na;
	ntfschar *attr_name;
	int attr_name_len = 0;
	ATTR_TYPES	 attribute=AT_DATA;

	attr_name = ntfs_str2ucs(NULL, &attr_name_len);
	if (!attr_name) {
		ntfs_log_perror("ERROR: Failed to parse attribute name = NULL"
			);
		goto end;
	}

	na = ntfs_attr_open(out, attribute, attr_name, attr_name_len);
	if (!na) {
		if (errno != ENOENT) {
			ntfs_log_perror("ERROR: Couldn't open attribute");
			goto end;
		}
		/* Requested attribute isn't present, add it. */
		if (ntfs_attr_add(out, attribute, attr_name,
			attr_name_len, NULL, 0)) {
				ntfs_log_perror("ERROR: Couldn't add attribute");
				goto end;
		}
		na = ntfs_attr_open(out, attribute, attr_name,
			attr_name_len);
		if (!na) {
			ntfs_log_perror("ERROR: Couldn't open just added "
				"attribute");
			goto end;
		}
	}
	ntfs_ucsfree(attr_name);

	ntfs_log_verbose("Old file size: %lld,new size=%lld\n", (long long)na->data_size,size);

	if (ntfs_attr_truncate_solid(na, size)) {
		ntfs_log_perror("ERROR: Couldn't resize attribute.newsize=%lld\n",size);
		goto close_attr;
	}

	if ((na->data_flags & ATTR_COMPRESSION_MASK)
		&& ntfs_attr_pclose(na))
		ntfs_log_perror("ERROR: ntfs_attr_pclose failed\n");

	ret = N_SUCCESS;
close_attr:
	ntfs_attr_close(na);

end:

	return ret;

}

// 获取文件大小
// NFILE file - 文件句柄
// long long *psize - 返回文件大小，byte
//
DWORD n_getfilesize(NFILE file,long long *psize)
{
	DWORD ret=N_FAIL;
	ntfs_inode *out=(ntfs_inode *)file;
	ntfs_attr *na;
	ntfschar *attr_name;
	int attr_name_len = 0;
	ATTR_TYPES	 attribute=AT_DATA;

	attr_name = ntfs_str2ucs(NULL, &attr_name_len);
	if (!attr_name) {
		ntfs_log_perror("ERROR: Failed to parse attribute name = NULL"
			);
		goto end;
	}

	na = ntfs_attr_open(out, attribute, attr_name, attr_name_len);
	if (!na) {
		ntfs_log_perror("ERROR: Couldn't open attribute");
	}
	ntfs_ucsfree(attr_name);

	ntfs_log_verbose("file size: %lld\n", (long long)na->data_size);

	if(psize)
	{
		*psize=na->data_size;
	}

	if ((na->data_flags & ATTR_COMPRESSION_MASK)
		&& ntfs_attr_pclose(na))
		ntfs_log_perror("ERROR: ntfs_attr_pclose failed\n");

	ret = N_SUCCESS;
// close_attr:
	ntfs_attr_close(na);

end:

	return ret;

}
