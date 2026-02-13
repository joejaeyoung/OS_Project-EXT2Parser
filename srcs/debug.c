#include "ssu_ext2.h"

/**
*
*tree 명령어 디버깅 출력 함수
*
*@param cmd 출력할 명령어 구조체
*/
void	debug_tree_cmd(Command cmd)
{
	printf("=== tree cmd ===\n");
	printf("cmd type : %s\n", cmd.cmd_type);
	printf("cmd path : %s\n", cmd.path);
	printf("cmd option : \n");
	printf("  -r : %s\n", cmd.options & TREE_OPT_R ? "On" : "Off");
	printf("  -s : %s\n", cmd.options & TREE_OPT_S ? "On" : "Off");
	printf("  -p : %s\n", cmd.options & TREE_OPT_P ? "On" : "Off");
}

/**
*
*print 명령어 디버깅 출력 함수
*
*@param cmd 출력할 명령어 구조체
*/
void	debug_print_cmd(Command cmd)
{
	printf("=== print cmd ===\n");
	printf("cmd type : %s\n", cmd.cmd_type);
	printf("cmd path : %s\n", cmd.path);
	printf("cmd option : \n");
	printf("  -n : %d\n", cmd.options);
	printf("    value : %d\n", cmd.extra_param);
}

/**
*
*디렉토리 엔트리 디버깅 함수 - 수정된 버전
*
*@param block_buf 디렉토리 블록 데이터 버퍼
*@param block_size 블록 크기
*/
void	debug_directory_block(unsigned char* block_buf, unsigned int block_size)
{
	printf("Full block analysis (showing entire block):\n");
		
	for (unsigned int offset = 0; offset < block_size; offset += 16) {
		// 16바이트씩 출력
		printf("%04x: ", offset);
		for (int i = 0; i < 16 && offset + i < block_size; i++) {
			printf("%02x ", block_buf[offset + i]);
			if (i == 7) printf(" ");  // 8바이트마다 공백 추가
		}
		
		// ASCII 표현 출력
		printf(" |");
		for (int i = 0; i < 16 && offset + i < block_size; i++) {
			char c = block_buf[offset + i];
			printf("%c", (c >= 32 && c <= 126) ? c : '.');
		}
		printf("|\n");
		
		// 가능한 디렉토리 엔트리 구조 분석
		if (offset + 8 <= block_size) {  // 최소 8바이트 필요
			struct my_ext2_dir_entry_2* entry = 
				(struct my_ext2_dir_entry_2*)(block_buf + offset);
			
			// 유효한 inode 번호인지 확인 (0이 아니고 일반적인 범위 내)
			if (entry->inode > 0 && entry->inode < 1000 &&
				entry->rec_len >= 8 && entry->rec_len <= 512 &&
				entry->name_len > 0 && entry->name_len < (unsigned char)256) {
				
				printf("  Possible entry: inode=%u, rec_len=%u, name_len=%u, file_type=%u\n", 
					  entry->inode, entry->rec_len, entry->name_len, entry->file_type);
				
				// 이름이 유효한 ASCII 문자인지 확인
				int valid_name = 1;
				for (int i = 0; i < entry->name_len && offset + 8 + i < block_size; i++) {
					char c = entry->name[i];
					if (c < 32 || c > 126) {
						valid_name = 0;
						break;
					}
				}
				
				if (valid_name && offset + 8 + entry->name_len <= block_size) {
					char name_buf[256];
					memset(name_buf, 0, sizeof(name_buf));
					strncpy(name_buf, entry->name, entry->name_len);
					printf("  Entry name: '%s'\n", name_buf);
				}
			}
		}
	}
}
