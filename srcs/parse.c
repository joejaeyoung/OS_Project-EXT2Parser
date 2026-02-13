#include "ssu_ext2.h"

/**
*
*print 명령어 파싱 함수
*
*@param line 사용자 입력 라인
*@param cmd 파싱 결과를 저장할 명령어 구조체 포인터
*@return 파싱 성공 시 true, 실패 시 false
*/
bool	parse_print_command(char *line, Command *cmd)
{
	char	*original_line = strdup(line);
	char	*argv[64] = {0};
	int		argc = 0;

	//문자열 토큰 분리
	char *token = strtok(line, " \t");
	while (token != NULL && argc < 64) {
		argv[argc++] = token;
		token = strtok(NULL, " \t");
	}

	if (argc < 1 || strcmp(argv[0], "print") != 0) {
		help_all();
		free(original_line);
		return false;
	}

	// 경로 인자 확인
	if (argc < 2) {
		//fprintf(stderr, "Error: Path not specified\n");
		help_all();
		free(original_line);
		return false;
	}

	strncpy(cmd->path, argv[1], sizeof(cmd->path) - 1);
	cmd->path[sizeof(cmd->path) - 1] = '\0';
	// check_path(cmd->path);
	if (cmd->path[strlen(cmd->path) - 1] == '/') {
		printf("Error: \'%s\' is not file\n", cmd->path);
		return false;
	}

	//옵션 파싱
	bool has_error = false;
	int n_flag = 0;
	for(int i = 2; i < argc; i++) {
		// 옵션은 -로 시작
		if (argv[i][0] != '-') {
			has_error = true;
			break ;
		}

		// 옵션의 길이 확인 최소 2 - + 하나 이상의 문자
		if (strlen(argv[i]) < 2) {
			has_error = true;
			break ;
		}

		if (strcmp(argv[i], "-n") == 0) {
			if (n_flag >= 1) {
					help_all();
					return false;
			}
			if (i +1 < argc) {
				//숫자 인지 확인
				char *endptr;
				long num = strtol(argv[i + 1], &endptr, 10);

				if (*endptr != '\0' || num <= 0) {
					has_error = true;
					break ;
				}

				cmd->extra_param = (int)num;
				n_flag++;
				i++; //숫자 인자 건너뛰기

			}
			else {
				printf("print: option requries an argument -- \'n\'\n");
				free(original_line);
				return false;
			}
		}
		else {
			help_all();
			return false;
		}
	}

	if (n_flag > 1) {
		help_all();
		free(original_line);
		return false;
	}

	if (has_error) {
		help_all();
		free(original_line);
		return false;
	}
	free(original_line);
	return true;
}

/**
*
*tree 명령어 파싱 함수
*
*@param line 사용자 입력 라인
*@param cmd 파싱 결과를 저장할 명령어 구조체 포인터
*@return 파싱 성공 시 true, 실패 시 false
*/
bool	parse_tree_command(char *line, Command *cmd)
{
	char	*original_line = strdup(line);
	char	*argv[64] = {0};
	int		argc = 0;

	//문자열 토큰 분리
	char *token = strtok(line, " \t");
	while (token != NULL && argc < 64) {
		argv[argc++] = token;
		token = strtok(NULL, " \t");
	}

	if (argc < 1 || strcmp(argv[0], "tree") != 0) {
		help_all();
		free(original_line);
		return (false);
	}

	// 경로 인자 확인
	if (argc < 2) {
		//fprintf(stderr, "Error: Path not specified\n");
		help_all();
		free(original_line);
		return false;
	}

	strncpy(cmd->path, argv[1], sizeof(cmd->path) - 1);
	cmd->path[sizeof(cmd->path) - 1] = '\0';

	//리턴값 int로 조정해서 0이면 help 출력 -1이면 내부 출력 1이면 성공
	int flag;
	if ((flag = validate_tree_path(cmd->path)) < 1) {
		if (flag == 0)
			help_all();
		free(original_line);
		return false;
	}

	//옵션 파싱
	bool has_error = false;
	int r_flag = 0;
	int s_flag = 0;
	int p_flag = 0;
	for(int i = 2; i < argc; i++) {
		// 옵션은 -로 시작
		if (argv[i][0] != '-') {
			has_error = true;
			break ;
		}

		// 옵션의 길이 확인 최소 2 - + 하나 이상의 문자
		if (strlen(argv[i]) < 2) {
			has_error = true;
			break ;
		}


		for (size_t j = 1; j < strlen(argv[i]); j++) {
			switch(argv[i][j]) {
				case 'r':
					cmd->options |= TREE_OPT_R;
					r_flag++;
					break;
				case 's':
					cmd->options |= TREE_OPT_S;
					s_flag++;
					break;
				case 'p':
					cmd->options |= TREE_OPT_P;
					p_flag++;
					break;
				default:
					has_error = true;
					break;
			}
			if (has_error) break;
		}
		if (has_error) break;
	}

	if (r_flag > 1 || s_flag > 1 || p_flag > 1) {
		help_all();
		free(original_line);
		return false;
	}

	if (has_error) {
		help_all();
		free(original_line);
		return false;
	}
	free(original_line);
	return true;
}
