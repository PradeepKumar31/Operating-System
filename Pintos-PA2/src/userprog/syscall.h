#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);

//Customized
int system_write(int file_desc,void *buf,int len);
int system_create(char *file,int init_size);
int system_open(char *file);
int system_seek(int file_desc,int position);
int system_filesize(int file_desc);
int system_tell(int file_desc);
int system_read(int file_desc,void *buf,int size);
struct file *find_file_by_file_desc(int file_desc);
struct filedesc_elem *find_filedesc_elem_by_file_desc(int file_desc);
struct filedesc_elem *find_filedesc_elem_by_file_desc_in_process(int file_desc);
struct list fl;
int system_exec(char *command);
int system_wait(int parent_id);
int system_remove(char *file);
int system_close(int file_desc);
int system_exit(int s);
///Customized

#endif /* userprog/syscall.h */
