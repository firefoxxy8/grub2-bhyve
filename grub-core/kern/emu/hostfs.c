/* hostfs.c - Dummy filesystem to provide access to the hosts filesystem  */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  GRUB is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  GRUB is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GRUB.  If not, see <http://www.gnu.org/licenses/>.
 */
#define _BSD_SOURCE
#include <grub/fs.h>
#include <grub/file.h>
#include <grub/disk.h>
#include <grub/misc.h>
#include <grub/mm.h>
#include <grub/dl.h>
#include <grub/util/misc.h>
#include <grub/emu/hostdisk.h>
#include <grub/i18n.h>

#include <dirent.h>
#include <stdio.h>
#include <errno.h>


/* dirent.d_type is a BSD extension, not part of POSIX */
#include <sys/stat.h>
#include <string.h>

static int
is_dir (const char *path, const char *name)
{
  int len1 = strlen(path);
  int len2 = strlen(name);

  char pathname[len1 + 1 + len2 + 1 + 13];
  strcpy (pathname, path);

  /* Avoid UNC-path "//name" on Cygwin.  */
  if (len1 > 0 && pathname[len1 - 1] != '/')
    strcat (pathname, "/");

  strcat (pathname, name);

  struct stat st;
  if (stat (pathname, &st))
    return 0;
  return S_ISDIR (st.st_mode);
}

struct grub_hostfs_data
{
  char *filename;
  FILE *f;
};

static grub_err_t
grub_hostfs_dir (grub_device_t device, const char *path,
		 int (*hook) (const char *filename,
			      const struct grub_dirhook_info *info))
{
  DIR *dir;

  /* Check if the disk is our dummy disk.  */
  if (grub_strcmp (device->disk->name, "host"))
    return grub_error (GRUB_ERR_BAD_FS, "not a hostfs");

  dir = opendir (path);
  if (! dir)
    return grub_error (GRUB_ERR_BAD_FILENAME,
		       N_("can't open `%s': %s"), path,
		       strerror (errno));

  while (1)
    {
      struct dirent *de;
      struct grub_dirhook_info info;
      grub_memset (&info, 0, sizeof (info));

      de = readdir (dir);
      if (! de)
	break;

      info.dir = !! is_dir (path, de->d_name);
      hook (de->d_name, &info);

    }

  closedir (dir);

  return GRUB_ERR_NONE;
}

/* Open a file named NAME and initialize FILE.  */
static grub_err_t
grub_hostfs_open (struct grub_file *file, const char *name)
{
  FILE *f;
  struct grub_hostfs_data *data;

  f = fopen (name, "rb");
  if (! f)
    return grub_error (GRUB_ERR_BAD_FILENAME,
		       N_("can't open `%s': %s"), name,
		       strerror (errno));
  data = grub_malloc (sizeof (*data));
  if (!data)
    {
      fclose (f);
      return grub_errno;
    }
  data->filename = grub_strdup (name);
  if (!data->filename)
    {
      grub_free (data);
      fclose (f);
      return grub_errno;
    }

  data->f = f;  

  file->data = data;

#ifdef __MINGW32__
  file->size = grub_util_get_disk_size (name);
#else
  file->size = grub_util_get_fd_size (fileno (f), name, NULL);
#endif

  return GRUB_ERR_NONE;
}

static grub_ssize_t
grub_hostfs_read (grub_file_t file, char *buf, grub_size_t len)
{
  struct grub_hostfs_data *data;

  data = file->data;
  if (fseeko (data->f, file->offset, SEEK_SET) != 0)
    {
      grub_error (GRUB_ERR_OUT_OF_RANGE, N_("cannot seek `%s': %s"),
		  data->filename, strerror (errno));
      return -1;
    }

  unsigned int s = fread (buf, 1, len, data->f);
  if (s != len)
    grub_error (GRUB_ERR_FILE_READ_ERROR, N_("cannot read `%s': %s"),
		data->filename, strerror (errno));

  return (signed) s;
}

static grub_err_t
grub_hostfs_close (grub_file_t file)
{
  struct grub_hostfs_data *data;

  data = file->data;
  fclose (data->f);
  grub_free (data->filename);
  grub_free (data);

  return GRUB_ERR_NONE;
}

static grub_err_t
grub_hostfs_label (grub_device_t device __attribute ((unused)),
		   char **label __attribute ((unused)))
{
  *label = 0;
  return GRUB_ERR_NONE;
}

static struct grub_fs grub_hostfs_fs =
  {
    .name = "hostfs",
    .dir = grub_hostfs_dir,
    .open = grub_hostfs_open,
    .read = grub_hostfs_read,
    .close = grub_hostfs_close,
    .label = grub_hostfs_label,
    .next = 0
  };



GRUB_MOD_INIT(hostfs)
{
  grub_fs_register (&grub_hostfs_fs);
}

GRUB_MOD_FINI(hostfs)
{
  grub_fs_unregister (&grub_hostfs_fs);
}
