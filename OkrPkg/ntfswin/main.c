/**
 * ntfscp - Part of the Linux-NTFS project.
 *
 * Copyright (c) 2004-2007 Yura Pakhuchiy
 * Copyright (c) 2005 Anton Altaparmakov
 * Copyright (c) 2006 Hil Liao
 *
 * This utility will copy file to an NTFS volume.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <signal.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include "types.h"
#include "attrib.h"
#include "utils.h"
#include "volume.h"
#include "dir.h"
#include "debug.h"
/* #include "version.h" */
#include "logging.h"
#include "disk.h"


/**
 * Create a regular file under the given directory inode
 *
 * It is a wrapper function to ntfs_create(...)
 *
 * Return:  the created file inode
 */
static ntfs_inode *ntfs_new_file(ntfs_inode *dir_ni,
			  const char *filename)
{
	ntfschar *ufilename;
	/* inode to the file that is being created */
	ntfs_inode *ni;
	int ufilename_len;

	/* ntfs_mbstoucs(...) will allocate memory for ufilename if it's NULL */
	ufilename = NULL;
	ufilename_len = ntfs_mbstoucs(filename, &ufilename);
	if (ufilename_len == -1) {
		ntfs_log_perror("ERROR: Failed to convert '%s' to unicode",
					filename);
		return NULL;
	}
	ni = ntfs_create(dir_ni, 0, ufilename, ufilename_len, S_IFREG);
	free(ufilename);
	return ni;
}

BOOL create_multidir_forfile(ntfs_volume *vol,char* path)
{
	char *p, *subdir;
	char* dir=strdup(path);
	ntfs_inode *parent_ni=NULL;
	ntfs_inode *sub_ni=NULL;

	if(!ISPATHSEPARATOR(*path))
	{
		ntfs_log_perror("path need begin with '\\'. %s\n",path);
		return FALSE;
	}

	if(dir==NULL)
	{
		ntfs_log_perror("strdup failed. %s\n",path);
		return FALSE;
	}

	//至少要4个字符以上 "\d\f"
	if(strlen(path)<4)
	{
		ntfs_log_perror("strlen(path)<4. %s\n",path);
		return FALSE;
	}

	parent_ni= ntfs_inode_open(vol, FILE_root);

	p=dir+1;
	subdir=dir+1;
	while(p!=dir+strlen(dir))
	{
		if(ISPATHSEPARATOR(*p))
		{
			*p='\0';		

			sub_ni = ntfs_pathname_to_inode(vol, NULL, dir);
			ntfs_log_debug("ntfs_pathname_to_inode,%p. %s\n",sub_ni,dir);
			if(sub_ni==NULL)
			{
// 				sub_ni=create_dir(parent_ni,subdir);

				if(sub_ni==NULL)
				{
					ntfs_log_perror("sub_ni==NULL. %s,%s\n",dir,subdir);
					goto freeres;
				}
			}
			else
			{
				ntfs_log_debug("sub_ni->mrec->flags=%u. %s\n",sub_ni->mrec->flags,dir);
				if(!sub_ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)
				{
					ntfs_log_perror("'%s' is not a directory.\n",dir);
					goto freeres;
				}
			}
			*p=PATH_SEP;
			subdir=p+1;
			ntfs_inode_close(parent_ni);
			parent_ni=sub_ni;
		}

		p++;
	}

freeres:
	if(dir)
		free(dir);
	if(sub_ni)
		ntfs_inode_close(sub_ni);
	if(parent_ni)
		ntfs_inode_close(parent_ni);

	return TRUE;
}

BOOL valid_filepath(char* path)
{
	char *lastp;

	if ((path == NULL) || (*path == '\0'))
		return FALSE;

	//最后一个字符不能是路径分隔符
	lastp = path + strlen(path) - 1;
	if(ISPATHSEPARATOR(*lastp))
		return FALSE;

	return TRUE;
}
#pragma warning(disable:4702)
int create_file(EFI_DISK_PARAMETER *efidisk,char* dest_file)
{
	ntfs_volume *vol;
	ntfs_inode *out;
	ntfs_attr *na;
	int flags = 0;
	int result = 1;
	s64 new_size;
	u64 offset;
	char *buf;
	s64 br, bw;
	ntfschar *attr_name;
	int attr_name_len = 0;
	ATTR_TYPES	 attribute=AT_DATA;

	if(!valid_filepath(dest_file))
	{
		ntfs_log_perror("ERROR: invalid file path. %s\n",dest_file);
		return 1;
	}

	vol = utils_mount_volume(efidisk, flags);
	if (!vol) {
		ntfs_log_perror("ERROR: couldn't mount volume");
		return 1;
	}

	NVolSetCompression(vol); /* allow compression */
	if (ntfs_volume_get_free_space(vol)) {
		ntfs_log_perror("ERROR: couldn't get free space");
		goto umount;
	}

	if(!create_multidir_forfile(vol,dest_file))
	{
		ntfs_log_perror("create_multidir_forfile failed.%s\n",dest_file);
		goto umount;
	}
	else
	{
		ntfs_log_debug("create_multidir_forfile succeed.%s\n",dest_file);
	}
	goto umount;

	new_size=4;

	out = ntfs_pathname_to_inode(vol, NULL, dest_file);

	if (!out) {
		/* Copy the file if the dest_file's parent dir can be opened. */
		char *parent_dirname;
		char *filename;
		ntfs_inode *dir_ni;
		ntfs_inode *ni;
		char *dirname_last_whack;

		filename = basename(dest_file);
		parent_dirname = strdup(dest_file);
		if (!parent_dirname) {
			ntfs_log_perror("strdup() failed");
			goto umount;
		}
		dirname_last_whack = strrchr(parent_dirname, PATH_SEP);
		if (dirname_last_whack) {
			dirname_last_whack[1] = 0;
			dir_ni = ntfs_pathname_to_inode(vol, NULL,
					parent_dirname);
		} else {
			ntfs_log_verbose("Target path does not contain PATH_SEP. "
					"Using root directory as parent.\n");
			dir_ni = ntfs_inode_open(vol, FILE_root);
		}
		if (dir_ni) {
			if (!(dir_ni->mrec->flags & MFT_RECORD_IS_DIRECTORY)) {
				/* Remove the last '/' for estetic reasons. */
				dirname_last_whack[0] = 0;
				ntfs_log_error("The file '%s' already exists "
						"and is not a directory. "
						"Aborting.\n", parent_dirname);
				free(parent_dirname);
				ntfs_inode_close(dir_ni);
				goto umount;
			}
			ntfs_log_verbose("Creating a new file '%s' under '%s'"
					 "\n", filename, parent_dirname);
			ni = ntfs_new_file(dir_ni, filename);
			ntfs_inode_close(dir_ni);
			if (!ni) {
				ntfs_log_perror("Failed to create '%s' under "
						"'%s'", filename,
						parent_dirname);
				free(parent_dirname);
				goto umount;
			}
			out = ni;
		} else {
			ntfs_log_perror("ERROR: Couldn't open '%s'",
					parent_dirname);
			free(parent_dirname);
			goto umount;
		}
		free(parent_dirname);
	}
	/* The destination is a directory. */
	if (out->mrec->flags & MFT_RECORD_IS_DIRECTORY) {
		char *filename;
		char *overwrite_filename;
		int overwrite_filename_len;
		ntfs_inode *ni;
		ntfs_inode *dir_ni;
		int filename_len;
		int dest_dirname_len;

		filename = basename(dest_file);
		dir_ni = out;
		filename_len = strlen(filename);
		dest_dirname_len = strlen(dest_file);
		overwrite_filename_len = filename_len+dest_dirname_len + 2;
		overwrite_filename = malloc(overwrite_filename_len);
		if (!overwrite_filename) {
			ntfs_log_perror("ERROR: Failed to allocate %i bytes "
					"memory for the overwrite filename",
					overwrite_filename_len);
			ntfs_inode_close(out);
			goto umount;
		}
		strcpy(overwrite_filename, dest_file);
		if (dest_file[dest_dirname_len - 1] != PATH_SEP) {
			strcat(overwrite_filename, "\\");
		}
		strcat(overwrite_filename, filename);
		ni = ntfs_pathname_to_inode(vol, NULL, overwrite_filename);
		/* Does a file with the same name exist in the dest dir? */
		if (ni) {
			ntfs_log_verbose("Destination path has a file with "
					"the same name\nOverwriting the file "
					"'%s'\n", overwrite_filename);
			ntfs_inode_close(out);
			out = ni;
		} else {
			ntfs_log_verbose("Creating a new file '%s' under "
					"'%s'\n", filename, dest_file);
			ni = ntfs_new_file(dir_ni, filename);
			ntfs_inode_close(dir_ni);
			if (!ni) {
				ntfs_log_perror("ERROR: Failed to create the "
						"destination file under '%s'",
						dest_file);
				free(overwrite_filename);
				goto umount;
			}
			out = ni;
		}
		free(overwrite_filename);
	}

	attr_name = ntfs_str2ucs(NULL, &attr_name_len);
	if (!attr_name) {
		ntfs_log_perror("ERROR: Failed to parse attribute name = NULL"
				);
		goto close_dst;
	}

	na = ntfs_attr_open(out, attribute, attr_name, attr_name_len);
	if (!na) {
		if (errno != ENOENT) {
			ntfs_log_perror("ERROR: Couldn't open attribute");
			goto close_dst;
		}
		/* Requested attribute isn't present, add it. */
		if (ntfs_attr_add(out, attribute, attr_name,
				attr_name_len, NULL, 0)) {
			ntfs_log_perror("ERROR: Couldn't add attribute");
			goto close_dst;
		}
		na = ntfs_attr_open(out, attribute, attr_name,
				attr_name_len);
		if (!na) {
			ntfs_log_perror("ERROR: Couldn't open just added "
					"attribute");
			goto close_dst;
		}
	}
	ntfs_ucsfree(attr_name);

	ntfs_log_verbose("Old file size: %lld\n", (long long)na->data_size);
	if (na->data_size != new_size) {
		if (ntfs_attr_truncate_solid(na, new_size)) {
			ntfs_log_perror("ERROR: Couldn't resize attribute");
			goto close_attr;
		}
	}

	buf = malloc(NTFS_BUF_SIZE);
	if (!buf) {
		ntfs_log_perror("ERROR: malloc failed");
		goto close_attr;
	}

	ntfs_log_verbose("Starting write.\n");
	offset = 0;
	{
		br = 4;
		strcpy(buf,"Nt");
		bw = ntfs_attr_pwrite(na, offset, br, buf);
		if (bw != br) {
			ntfs_log_perror("ERROR: ntfs_attr_pwrite failed");
//			break;
		}
		offset += bw;
	}
	if ((na->data_flags & ATTR_COMPRESSION_MASK)
	    && ntfs_attr_pclose(na))
		ntfs_log_perror("ERROR: ntfs_attr_pclose failed");
	ntfs_log_verbose("Syncing.\n");
	result = 0;
	free(buf);
close_attr:
	ntfs_attr_close(na);
close_dst:
	while (ntfs_inode_close(out)) {
		if (errno != EBUSY) {
			ntfs_log_error("Sync failed. Run chkdsk.\n");
			break;
		}
		ntfs_log_error("Device busy.  Will retry sync in 3 seconds.\n");
		sleep(3);
	}

umount:
	ntfs_umount(vol, FALSE);
	ntfs_log_verbose("Done.\n");
	return result;
}

#include "ntfs.h"

int testcreatedir(char* dirpath)
{
	DWORD ret=N_SUCCESS;

	ret=n_createdir(dirpath);
	if(NSUCCEED(ret))
	{
		printf("create dir '%s' succeed.\n",dirpath);
	}
	else
	{
		printf("create dir '%s' failed.ret=0x%lx,%lu\n",dirpath,ret,ret);
	}
	return 0;
}

int testremovedir(char* dirpath)
{
	DWORD ret=N_SUCCESS;

	ret=n_deletedir(dirpath);
	if(NSUCCEED(ret))
	{
		printf("delete dir '%s' succeed.\n",dirpath);
	}
	else
	{
		printf("delete dir '%s' failed.ret=0x%lx,%lu\n",dirpath,ret,ret);
	}
	return 0;
}

int testlistdir(char* dirpath)
{
	int i;
	s64 size=1024*1024;
	void* buf=malloc(size);
	DIR_LIST* pdirlist=(DIR_LIST*)buf;
	n_listdir(dirpath,&size,buf);

	for(i=0;i<pdirlist->count;i++)
	{
		struct tm *t;
		char type[2];
		t=localtime(&(pdirlist->entry[i].modifytime));

		switch(pdirlist->entry[i].type)
		{
		case N_TYPE_DIR:
			strncpy(type,"D",sizeof(type));
			break;
		case N_TYPE_FILE:
			strncpy(type,"F",sizeof(type));
			break;
		default:
			strncpy(type,"U",sizeof(type));
			break;
		}
		printf("%30s,%s,%10lld,%04d-%02d-%02d %02d:%02d:%02d\n",
			pdirlist->entry[i].name,
			type,
			pdirlist->entry[i].filesize,
			t->tm_year+1900,
			t->tm_mon,
			t->tm_mday,
			t->tm_hour,
			t->tm_min,
			t->tm_min);
	}
	free(buf);

	return 0;
}

int testcreatefile(char* path)
{
	DWORD ret=N_SUCCESS;
	NFILE file=NULL;
	ret=n_createfile(path,&file);
	if(NSUCCEED(ret)&&file)
	{
		printf("create file succeed.'%s'\n",path);
		n_closefile(file);
	}
	else
	{
		printf("create file failed.'%s',ret=0x%lx,%lu\n",path,ret,ret);
	}
	return 0;
}

int testdeletefile(char* path)
{
	DWORD ret=N_SUCCESS;
	
	ret=n_deletefile(path);
	if(NSUCCEED(ret))
	{
		printf("delete file succeed.'%s'\n",path);
	}
	else
	{
		printf("delete file failed.'%s',ret=0x%lx,%lu\n",path,ret,ret);
	}

	return 0;
}

int testreadfile(char* path)
{
	DWORD ret=N_SUCCESS;
	NFILE file=NULL;
	void *buf=NULL;
	long long size=0;
	long long br;

	ret=n_openfile(path,&file);

	if(NSUCCEED(ret)&&file)
	{
		ret=n_getfilesize(file,&size);
		if(NSUCCEED(ret))
		{
			buf=malloc(size);

			if(buf)
			{
				ret=n_readfile(file,0,size,buf,&br);

				if(NSUCCEED(ret))
				{
					printf("'%s' read size=%lld\n",path,size);
				}
				else
				{
					printf("'%s' write file failed.ret=0x%lx,%lu\n",path,ret,ret);
				}
			}
			else
			{
				printf("buf==null\n");
			}
		}
		else
		{
			printf("'%s' get file size failed.ret=0x%lx,%lu\n",path,ret,ret);
		}

		n_closefile(file);
	}
	else
	{
		printf("open file %s failed.ret=0x%lx,%lu\n",path,ret,ret);
	}

	if(buf)
		free(buf);

	return 0;
}

int testwritefile(char* path)
{
	DWORD ret=N_SUCCESS;
	NFILE file=NULL;
	int bufsize=1024*64;
	char *buf=malloc(bufsize);
	int i,j;
	int max=100;
	long long size,total=0;
	long long bw;

	printf("create file '%s'\n",path);

	ret=n_createfile(path,&file);
	if(!NSUCCEED(ret))
	{
		printf("open file '%s'\n",path);
		ret=n_openfile(path,&file);
	}

	if(NSUCCEED(ret)&&file)
	{
		srand(time(NULL));
		for(i=0;i<max;i++)
		{
			size=rand()%max;
			for(j=0;j<size;j++)
			{
				buf[j]='0'+(rand()%10);
			}
			ret=n_writefile(file,total,size,buf,&bw);

			if(NSUCCEED(ret))
			{
				total+=size;
				printf("'%s' write totalsize=%lld\n",path,total);
			}
			else
			{
				printf("'%s' write file failed.ret=0x%lx,%lu\n",path,ret,ret);
			}
		}
		n_closefile(file);
	}
	else
	{
		printf("open file %s failed.ret=0x%lx,%lu,file=%p\n",path,ret,ret,file);
	}

	if(buf)
		free(buf);

	return 0;
}

int testgetfilesize(char* path)
{
	DWORD ret=N_SUCCESS;
	NFILE file=NULL;
	long long size=0;
	ret=n_openfile(path,&file);
	if(NSUCCEED(ret)&&file)
	{
		ret=n_getfilesize(file,&size);
		if(NSUCCEED(ret))
		{
			printf("'%s' file size=%lld\n",path,size);
		}
		else
		{
			printf("'%s' get file size failed.ret=0x%lx,%lu\n",path,ret,ret);
		}
		n_closefile(file);
	}
	else
	{
		printf("open file %s failed.ret=0x%lx,%lu\n",path,ret,ret);
	}
	return 0;
}

int testsetfilesize(char* path,char* szsize)
{
	DWORD ret=N_SUCCESS;
	NFILE file=NULL;
	long long size=atoi(szsize);
	ret=n_openfile(path,&file);
	if(NSUCCEED(ret)&&file)
	{
		ret=n_setfilesize(file,size);
		if(NSUCCEED(ret))
		{
			printf("'%s' file size=%lld\n",path,size);
		}
		else
		{
			printf("'%s' set file size failed.ret=0x%lx,%lu\n",path,ret,ret);
		}
		n_closefile(file);
	}
	else
	{
		printf("open file %s failed.ret=0x%lx,%lu\n",path,ret,ret);
	}
	return 0;
}


/**
 * main - Begin here
 *
 * Start from here.
 *
 * Return:  0  Success, the program worked
 *	    1  Error, something went wrong
 */
int main(int argc, char *argv[])
{
// 	DWORD ret=N_SUCCESS;
	int i;
	printf("ntfs begin,argc=%d\n",argc);

	for(i=0;i<argc;i++)
	{
		printf("argv[%d]=%s\n",i,argv[i]);
	}

	if(argc<3)
		return 0;

	BlockIoInit();

	if(gDisk.Count<=0){
		ntfs_log_perror("ERROR: gDisk.Count<=0");
		return 1;
	}

	n_init(&gDisk.Disk[0],FALSE);

	if(strcmp(argv[1],"-md")==0)
	{
		testcreatedir(argv[2]);
	}
	else if(strcmp(argv[1],"-rd")==0)
	{
		testremovedir(argv[2]);
	}
	else if(strcmp(argv[1],"-ls")==0)
	{
		testlistdir(argv[2]);
	}
	else if(strcmp(argv[1],"-c")==0)
	{
		testcreatefile(argv[2]);
	}
	else if(strcmp(argv[1],"-d")==0)
	{
		testdeletefile(argv[2]);
	}
	else if(strcmp(argv[1],"-r")==0)
	{
		testreadfile(argv[2]);
	}
	else if(strcmp(argv[1],"-w")==0)
	{
		testwritefile(argv[2]);
	}
	else if(strcmp(argv[1],"-g")==0)
	{
		testgetfilesize(argv[2]);
	}
	else if(strcmp(argv[1],"-s")==0)
	{
		testsetfilesize(argv[2],argv[3]);
	}

	n_uninit(TRUE);

	BlockIoRelease();

	printf("ntfs exit\n");

	return 0;

}

// struct options {
// 	char		*device;	/* Device/File to work with */
// 	char		*src_file;	/* Source file */
// 	char		*dest_file;	/* Destination file */
// 	char		*attr_name;	/* Write to attribute with this name. */
// 	int		 force;		/* Override common sense */
// 	int		 quiet;		/* Less output */
// 	int		 verbose;	/* Extra output */
// 	int		 noaction;	/* Do not write to disk */
// 	ATTR_TYPES	 attribute;	/* Write to this attribute. */
// 	int		 inode;		/* Treat dest_file as inode number. */
// };
// 
// static const char *EXEC_NAME = "ntfs";
// static struct options opts;
// static volatile sig_atomic_t caught_terminate = 0;
// #define VERSION "2014.3.1"
// 
// /**
//  * version - Print version information about the program
//  *
//  * Print a copyright statement and a brief description of the program.
//  *
//  * Return:  none
//  */
// static void version(void)
// {
// 	ntfs_log_info("\n%s v%s Ntfs Uefi Read Write Function"
// 		".\n\n", EXEC_NAME, VERSION);
// 	ntfs_log_info("Copyright (c) 2014 Liyi\n");
// // 	ntfs_log_info("\n%s\n%s%s\n", ntfs_gpl, ntfs_bugs, ntfs_home);
//}
// 
// /**
//  * usage - Print a list of the parameters to the program
//  *
//  * Print a list of the parameters and options for the program.
//  *
//  * Return:  none
//  */
// static void usage(void)
// {
// 	ntfs_log_info("\nUsage: %s [options] device src_file dest_file\n\n"
// 		"    -m, --createdir         Create directory\n"
// 		"    -n, --removedir         Remove directory\n"
// 		"    -l, --listdir			  List directory\n"
// 		"    -c, --createfile		  Create file\n"
// 		"    -d, --deletefile		  Delete file\n"
// 		"    -r, --readfile		      Read file\n"
// 		"    -w, --writefile		  Write file\n"
// 		"    -g, --getfilesize		  Get file size\n"
// 		"    -s, --setfilesize		  Set file size\n"
// 		"    -V, --version         Version information\n",
// 		EXEC_NAME);
// 	ntfs_log_info("%s%s\n", ntfs_bugs, ntfs_home);
// }
// 
// /**
//  * parse_options - Read and validate the programs command line
//  *
//  * Read the command line, verify the syntax and parse the options.
//  * This function is very long, but quite simple.
//  *
//  * Return:  1 Success
//  *	    0 Error, one or more problems
//  */
// static int parse_options(int argc, char **argv)
// {
// 	static const char *sopt = "-mnlcdrwgshV";
// 	static const struct option lopt[] = {
// 		{ "createdir",	required_argument,	NULL, 'm' },
// 		{ "removedir",	required_argument,	NULL, 'n' },
// 		{ "listdir",	required_argument,	NULL, 'l' },
// 		{ "createfile",	required_argument,	NULL, 'c' },
// 		{ "deletefile",	required_argument,	NULL, 'd' },
// 		{ "readfile",	required_argument,	NULL, 'r' },
// 		{ "writefile",	required_argument,	NULL, 'w' },
// 		{ "getfilesize",	required_argument,	NULL, 'g' },
// 		{ "setfilesize",	required_argument,	NULL, 's' },
// 		{ "help",	no_argument,		NULL, 'h' },
// 		{ "version",	no_argument,		NULL, 'V' },
// 		{ NULL,		0,			NULL, 0   }
// 	};
// 
// 	char *s;
// 	int c = -1;
// 	int err  = 0;
// 	int ver  = 0;
// 	int help = 0;
// 	int levels = 0;
// 	s64 attr;
// 
// 	opts.device = NULL;
// 	opts.src_file = NULL;
// 	opts.dest_file = NULL;
// 	opts.attr_name = NULL;
// 	opts.inode = 0;
// 	opts.attribute = AT_DATA;
// 
// 	opterr = 0; /* We'll handle the errors, thank you. */
// 
// 	while ((c = getopt_long(argc, argv, sopt, lopt, NULL)) != -1) {
// 		switch (c) {
// 		case 1:	/* A non-option argument */
// 			if (!opts.device) {
// 				opts.device = argv[optind - 1];
// 			} else if (!opts.src_file) {
// 				opts.src_file = argv[optind - 1];
// 			} else if (!opts.dest_file) {
// 				opts.dest_file = argv[optind - 1];
// 			} else {
// 				ntfs_log_error("You must specify exactly two "
// 						"files.\n");
// 				err++;
// 			}
// 			break;
// 		case 'a':
// 			if (opts.attribute != AT_DATA) {
// 				ntfs_log_error("You can specify only one "
// 						"attribute.\n");
// 				err++;
// 				break;
// 			}
// 
// 			attr = strtol(optarg, &s, 0);
// 			if (*s) {
// 				ntfs_log_error("Couldn't parse attribute.\n");
// 				err++;
// 			} else
// 				opts.attribute = (ATTR_TYPES)cpu_to_le32(attr);
// 			break;
// 		case 'i':
// 			opts.inode++;
// 			break;
// 		case 'f':
// 			opts.force++;
// 			break;
// 		case 'h':
// 		case '?':
// 			if (strncmp(argv[optind - 1], "--log-", 6) == 0) {
// 				if (!ntfs_log_parse_option(argv[optind - 1]))
// 					err++;
// 				break;
// 			}
// 			help++;
// 			break;
// 		case 'N':
// 			if (opts.attr_name) {
// 				ntfs_log_error("You can specify only one "
// 						"attribute name.\n");
// 				err++;
// 			} else
// 				opts.attr_name = argv[optind - 1];
// 			break;
// 		case 'n':
// 			opts.noaction++;
// 			break;
// 		case 'q':
// 			opts.quiet++;
// 			ntfs_log_clear_levels(NTFS_LOG_LEVEL_QUIET);
// 			break;
// 		case 'V':
// 			ver++;
// 			break;
// 		case 'v':
// 			opts.verbose++;
// 			ntfs_log_set_levels(NTFS_LOG_LEVEL_VERBOSE);
// 			break;
// 		default:
// 			ntfs_log_error("Unknown option '%s'.\n",
// 					argv[optind - 1]);
// 			err++;
// 			break;
// 		}
// 	}
// 
// 	/* Make sure we're in sync with the log levels */
// 	levels = ntfs_log_get_levels();
// 	if (levels & NTFS_LOG_LEVEL_VERBOSE)
// 		opts.verbose++;
// 	if (!(levels & NTFS_LOG_LEVEL_QUIET))
// 		opts.quiet++;
// 
// 	if (help || ver) {
// 		opts.quiet = 0;
// 	} else {
// 		if (!opts.device) {
// 			ntfs_log_error("You must specify a device.\n");
// 			err++;
// 		} else if (!opts.src_file) {
// 			ntfs_log_error("You must specify a source file.\n");
// 			err++;
// 		} else if (!opts.dest_file) {
// 			ntfs_log_error("You must specify a destination "
// 					"file.\n");
// 			err++;
// 		}
// 
// 		if (opts.quiet && opts.verbose) {
// 			ntfs_log_error("You may not use --quiet and --verbose "
// 					"at the same time.\n");
// 			err++;
// 		}
// 	}
// 
// 	if (ver)
// 		version();
// 	if (help || err)
// 		usage();
// 
// 	return (!err && !help && !ver);
// }
// 
// /**
//  * main - Begin here
//  *
//  * Start from here.
//  *
//  * Return:  0  Success, the program worked
//  *	    1  Error, something went wrong
//  */
// int main(int argc, char *argv[])
// {
// 
// 	if (!parse_options(argc, argv))
// 		return 1;
// 
// 	return 0;
// }
