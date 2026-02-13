#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>

#include "ext2.h"

#define MAX_PATH 4096
#define MAX_FILE_NAME 255
#define BUFFER_SIZE 4096

#define TREE_OPT_R 0x01
#define TREE_OPT_S 0x02
#define TREE_OPT_P 0x04

typedef struct command {
	char	cmd_type[10];
	char	path[4096];
	int		options;
	int		extra_param;
} Command;

/**
 * 디렉토리 트리 노드 구조체 (링크드 리스트 구현)
 */
typedef struct dir_tree_node {
	char name[MAX_FILE_NAME + 1];	  // 파일/디렉토리 이름
	int inode_num;					 // inode 번호
	int file_type;					 // 파일 타입 (S_IFDIR, S_IFREG 등)
	unsigned int size;				 // 파일 크기
	unsigned int permissions;		  // 권한 정보
		
	struct dir_tree_node *first_child; // 첫 번째 자식 노드 (디렉토리인 경우)
	struct dir_tree_node *next_sibling; // 다음 형제 노드
} DirTreeNode;

extern char *img_path;

/* debug.c */
void	debug_tree_cmd(Command cmd);
void	debug_print_cmd(Command cmd);
void	debug_directory_block(unsigned char* block_buf, unsigned int block_size);

/* ext2_utils.c */
int read_super_block(int fd, struct my_ext2_super_block *sb);
unsigned int get_block_size(struct my_ext2_super_block *sb);
int read_data_block(int fd, struct my_ext2_super_block *sb, unsigned int block_num, unsigned char *buffer);

/* ext2_inode.c */
unsigned int path_to_inode(int fd, struct my_ext2_super_block *sb, 
						  struct my_ext2_group_desc *gd, 
						  const char *path);
unsigned int find_entry_in_dir(int fd, struct my_ext2_super_block *sb, 
							  struct my_ext2_inode *dir_inode, 
							  const char *name);
int read_inode(int fd, int inode_num, 
			  struct my_ext2_super_block *sb, 
			  struct my_ext2_group_desc *gd, 
			  struct my_ext2_inode *inode);
			  
/* help.c */
void	help(char *line);
void	help_all();
void	help_tree();
void	help_print();
void	help_help();
void	help_exit();

/* parse.c */
bool	parse_tree_command(char *line, Command *cmd);
bool	parse_print_command(char *line, Command *cmd); 

/* print.c */
void print(Command *cmd);
int print_file_content(int fd, struct my_ext2_super_block *sb, 
					  struct my_ext2_inode *inode, 
					  int line_count);

/* tree.c */
void count_files_and_dirs(DirTreeNode* node, int* file_count, int* dir_count);
void tree(Command *cmd);
DirTreeNode* create_tree_node(const char* name, int inode_num, int file_type, unsigned int size, unsigned int permissions);
void free_tree_node(DirTreeNode* node);
int read_directory_entries(int fd, struct my_ext2_super_block *sb, 
						  struct my_ext2_group_desc *gd,
						  unsigned int dir_inode_num, DirTreeNode *parent_node, 
						  int recursive);
int process_directory_block(int fd, struct my_ext2_super_block *sb, 
						   struct my_ext2_group_desc *gd,
						   unsigned int block_num, DirTreeNode *parent_node,
						   int recursive, int *dir_count, int *file_count);
int process_indirect_block(int fd, struct my_ext2_super_block *sb, 
						  struct my_ext2_group_desc *gd,
						  unsigned int indirect_block_num, DirTreeNode *parent_node,
						  int recursive, int *dir_count, int *file_count);
int process_double_indirect_block(int fd, struct my_ext2_super_block *sb, 
								 struct my_ext2_group_desc *gd,
								 unsigned int double_indirect_block_num, 
								 DirTreeNode *parent_node, int recursive, 
								 int *dir_count, int *file_count);
int process_triple_indirect_block(int fd, struct my_ext2_super_block *sb, 
								 struct my_ext2_group_desc *gd,
								 unsigned int triple_indirect_block_num, 
								 DirTreeNode *parent_node, int recursive, 
								 int *dir_count, int *file_count);

/* utils */
char	**fix_split(char const *s, char c);

/* validate.c */
int validate_tree_path(const char *path);
