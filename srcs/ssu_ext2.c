#include "ssu_ext2.h"

char *img_path;

/**
*
*사용자 입력을 받는 함수
*
*@return 입력받은 문자열 포인터
*/
char	*get_input_line()
{
	char	*line;
	size_t	size;
	size_t	input_len;

	size = MAX_PATH * 2;
	line = (char *)malloc(sizeof(char) * size);
	
	while(1) {
		printf("20201505> ");
		memset(line, 0, sizeof(char) * size);
		if (fgets(line, size, stdin) != NULL) {
			input_len = strlen(line);

			if (input_len > 0 && line[input_len - 1] == '\n') {
				line[input_len - 1] = '\0';
			}

			//엔터 무시
			if (strlen(line) != 0) {
				break;
			}
		}
	}

	return (line);
}

/**
*
*EXT2 파일 시스템 슈퍼블록 확인 함수
*
*@param str 확인할 파일 경로
*@return EXT2 파일 시스템이면 true, 아니면 false
*/
bool	check_super_magic(char *str)
{
	int	fd;
	struct my_ext2_super_block sb;

	if ((fd = open(str, O_RDONLY)) < 0) {
		fprintf(stdout, "file open error\n");
		exit(1);
	}

	lseek(fd, 1024, SEEK_SET);

	if (read(fd, &sb, sizeof(struct my_ext2_super_block)) != sizeof(struct my_ext2_super_block)) {
		fprintf(stdout, "read error\n");
		exit(1);
	}

	//super block 확인
	if (sb.s_magic == EXT2_SUPER_MAGIC) {
		return true;
	}	
	return false;
}

/**
 * '~/'로 시작하는 경로를 홈 디렉토리 경로로 확장하고 절대 경로로 변환하는 함수
 * 
 * @param path 변환할 경로
 * @return 절대 경로 (호출자가 free 해야 함)
 */
char* get_absolute_path(const char* path)
{
	if (path == NULL) {
		return NULL;
	}
		
	char expanded_path[MAX_PATH];
		
	// 홈 디렉토리(~/) 처리
	if (path[0] == '~' && (path[1] == '/' || path[1] == '\0')) {
		char* home = getenv("HOME");
		
		// 홈 디렉토리 경로와 '~/' 이후의 나머지 경로 합치기
		snprintf(expanded_path, MAX_PATH, "%s%s", home, path + 1);
	} else {
		strncpy(expanded_path, path, MAX_PATH - 1);
		expanded_path[MAX_PATH - 1] = '\0';
	}
		
	// 절대 경로로 변환
	char* absolute_path = (char*)malloc(MAX_PATH);
		
	if (realpath(expanded_path, absolute_path) == NULL) {
		free(absolute_path);
		return NULL;
	}
		
	return absolute_path;
}

/**
*
*프로그램 메인 함수
*
*@param argc 명령행 인자 개수
*@param argv 명령행 인자 배열
*@return 프로그램 종료 코드
*/
int	main(int argc, char *argv[])
{
	char	*line;
	char	*imgfile_path;

	//todo : 에러 처리 
	if (argc != 2) {
		printf("Usage Error : ./ssu_ext2 <EXT2_IMAGE>\n");
		exit(0);
	}

	imgfile_path = get_absolute_path(argv[1]);
	if (!imgfile_path) {
		printf("Usage Error : ./ssu_ext2 <EXT2_IMAGE>\n");
		exit(0);
	}

	if (!check_super_magic(imgfile_path)) {
		printf("Error : bad file system\n");
		exit(0);
	}
	img_path = strdup(imgfile_path);

	Command cmd;
	while (true) {
		memset(&cmd, 0, sizeof(Command));
		line = get_input_line();

		if (!strncmp(line, "help", 4)) {
			help(line);
		}
		else if (!strcmp(line, "exit")) {
			free(line);
			exit(0);
		}
		else if (!strncmp(line, "tree", 4)) {
			if (parse_tree_command(line, &cmd)) {
				#ifdef DEBUG_CMD
					debug_tree_cmd(cmd);
				#endif
				tree(&cmd);
			}
		}
		else if (!strncmp(line, "print", 5)) {
			if (parse_print_command(line, &cmd)) {
				#ifdef DEBUG_CMD
					debug_print_cmd(cmd);
				#endif
				print(&cmd);
			}
		}
		else {
			help_all();
		}
		free(line);
	}
}
