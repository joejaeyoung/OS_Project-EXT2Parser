#include "ssu_ext2.h"

/**
*
*사용자 입력에 따른 도움말 출력 함수
*
*@param line 사용자 입력 라인
*/
void	help(char *line)
{
	int		argc;
	char	**splited;
	
	splited = fix_split(line, ' ');
	for(int i = 0; splited[i]; i++)
		argc++;
	
	#ifdef DEBUG_HELP
		for(int i = 0; splited[i]; i++) {
			printf("%d:%s\n", i, splited[i]);
		}
		printf("argc  %d\n", argc);
	#endif
	if (argc > 2) {
		if (!strcmp(splited[0], "help")) {
			printf("invalid command -- \'");
			for(int i = 1; i < argc; i++) {
				if (i != 1)
					printf(" ");
				printf("%s", splited[i]);
			}
			printf("\'\n");
		}
		#ifdef DEBUG_HELP
			printf("잘못된 help 입력\n");
		#endif
		help_all();
		return ;
	}

	if (argc == 1) {
		#ifdef DEBUG_HELP
			printf("help만 들어왔을 경우\n");
		#endif
		help_all();
		return ;
	}

	if (!strcmp(splited[1], "tree")) {
		#ifdef DEBUG_HELP
			printf("help tree\n");
		#endif
		help_tree();
		return ;
	}

	if (!strcmp(splited[1], "print")) {
		#ifdef DEBUG_HELP
			printf("help print\n");
		#endif
		help_print();
		return ;
	}
	
	if (!strcmp(splited[1], "help")) {
		#ifdef DEBUG_HELP
			printf("help help\n");
		#endif
		help_help();
		return ;
	}

	if (!strcmp(splited[1], "exit")) {
		#ifdef DEBUG_HELP
			printf("help exit\n");
		#endif
		help_exit();
		return ;
	}

	printf("invalid command -- \'%s\'\n", splited[1]);
	help_all();
}

/**
*
*모든 명령어에 대한 도움말 출력 함수
*/
void	help_all()
{
	printf("Usage:\n");
	printf("  > tree <PATH> [OPTION]... : display the directory structure if <PATH> is a directory\n");
	printf("    -r : display the directory structure recursively if <PATH> is a directory\n");
	printf("    -s : display the directory structure if <PATH> is a directory, including the size of each file\n");
	printf("    -p : display the directory structure if <PATH> is a directory, including the permissions of each directory and file\n");
	printf("  > print <PATH> [OPTION]... : print the contents on the standard output if <PATH> is file\n");
	printf("    -n <line_number> : print only the first <line_number> lines of its contents on the standard output if <PATH> is file\n");
	printf("  > help [COMMAND] : show commands for progarm\n");
	printf("  > exit : exit program\n");
}

/**
*
*tree 명령어 도움말 출력 함수
*/
void	help_tree()
{
	printf("Usage:\n");
	printf("  > tree <PATH> [OPTION]... : display the directory structure if <PATH> is a directory\n");
	printf("    -r : display the directory structure recursively if <PATH> is a directory\n");
	printf("    -s : display the directory structure if <PATH> is a directory, including the size of each file\n");
	printf("    -p : display the directory structure if <PATH> is a directory, including the permissions of each directory and file\n");
}

/**
*
*print 명령어 도움말 출력 함수
*/
void	help_print()
{
	printf("Usage:\n");
	printf("  > print <PATH> [OPTION]... : print the contents on the standard output if <PATH> is file\n");
	printf("    -n <line_number> : print only the first <line_number> lines of its contents on the standard output if <PATH> is file\n");
}

/**
*
*help 명령어 도움말 출력 함수
*/
void	help_help()
{
	printf("Usage:\n");
	printf("  > help [COMMAND] : show commands for progarm\n");
}

/**
*
*exit 명령어 도움말 출력 함수
*/
void	help_exit()
{
	printf("Usage:\n");
	printf("  > exit : exit program\n");
}
