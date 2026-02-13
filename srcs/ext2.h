#ifndef EXT2_H
# define EXT2_H

#include <sys/types.h>
#include <stdint.h>

#ifndef __u8
typedef uint8_t  __u8;
#endif
#ifndef __u16
typedef uint16_t __u16;
#endif
#ifndef __u32
typedef uint32_t __u32;
#endif
// 이 두 줄이 문제를 일으키므로 제거하거나 수정
// typedef uint64_t __u64;  <- 문제 발생!
// typedef int64_t  __s64;  <- 문제 발생!

#ifndef __s8
typedef int8_t   __s8;
#endif
#ifndef __s16
typedef int16_t  __s16;
#endif
#ifndef __s32
typedef int32_t  __s32;
#endif

// 파일 타입
#define EXT2_FT_UNKNOWN     0
#define EXT2_FT_REG_FILE    1
#define EXT2_FT_DIR         2
#define EXT2_FT_CHRDEV      3
#define EXT2_FT_BLKDEV      4
#define EXT2_FT_FIFO        5
#define EXT2_FT_SOCK        6
#define EXT2_FT_SYMLINK     7

#define EXT2_SUPER_MAGIC       0xEF53  // 슈퍼블록 매직 넘버
#define EXT2_ROOT_INO          2       // 루트 디렉토리 inode 번호
#define EXT2_NDIR_BLOCKS       12      // 직접 블록 개수
#define EXT2_IND_BLOCK         12      // 단일 간접 블록 인덱스
#define EXT2_DIND_BLOCK        13      // 이중 간접 블록 인덱스
#define EXT2_TIND_BLOCK        14      // 삼중 간접 블록 인덱스
#define EXT2_N_BLOCKS          15      // i_block 배열 크기
// // 파일 모드 (i_mode) 매크로
// #define S_IFMT      0xF000  // 파일 타입 마스크
// #define S_IFREG     0x8000  // 일반 파일
// #define S_IFDIR     0x4000  // 디렉토리
// #define S_IFCHR     0x2000  // 문자 디바이스
// #define S_IFBLK     0x6000  // 블록 디바이스
// #define S_IFIFO     0x1000  // FIFO
// #define S_IFSOCK    0xC000  // 소켓
// #define S_IFLNK     0xA000  // 심볼릭 링크

// // 파일 모드 확인 매크로
// #define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
// #define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
// #define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
// #define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
// #define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
// #define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
// #define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)

struct my_ext2_super_block {
    __u32   s_inodes_count;      /* 아이노드 개수 */
    __u32   s_blocks_count;      /* 블록 개수 */
    __u32   s_r_blocks_count;    /* 예약된 블록 개수 */
    __u32   s_free_blocks_count; /* 여유 블록 개수 */
    __u32   s_free_inodes_count; /* 여유 아이노드 개수 */
    __u32   s_first_data_block;  /* 첫 번째 데이터 블록 번호 */
    __u32   s_log_block_size;    /* 블록 크기 (1024 << n) */
    __u32   s_log_frag_size;     /* 단편화 크기 */
    __u32   s_blocks_per_group;  /* 블록 그룹당 블록 수 */
    __u32   s_frags_per_group;   /* 블록 그룹당 단편화 수 */
    __u32   s_inodes_per_group;  /* 블록 그룹당 아이노드 수 */
    __u32   s_mtime;             /* 마운트 시간 */
    __u32   s_wtime;             /* 쓰기 시간 */
    __u16   s_mnt_count;         /* 마운트 횟수 */
    __s16   s_max_mnt_count;     /* 최대 마운트 횟수 */
    __u16   s_magic;             /* 매직 넘버 (0xEF53) */
    __u16   s_state;             /* 파일 시스템 상태 */
    __u16   s_errors;            /* 오류 발생 시 동작 */
    __u16   s_minor_rev_level;   /* 마이너 버전 */
    __u32   s_lastcheck;         /* 마지막 체크 시간 */
    __u32   s_checkinterval;     /* 체크 간격 */
    __u32   s_creator_os;        /* 생성 OS */
    __u32   s_rev_level;         /* 리비전 레벨 */
    __u16   s_def_resuid;        /* 기본 예약 UID */
    __u16   s_def_resgid;        /* 기본 예약 GID */
    
    /* EXT2_DYNAMIC_REV 슈퍼블록에만 유효한 필드 */
    __u32   s_first_ino;         /* 첫 번째 비예약 아이노드 */
    __u16   s_inode_size;        /* 아이노드 크기 */
    __u16   s_block_group_nr;    /* 이 슈퍼블록의 블록 그룹 번호 */
    __u32   s_feature_compat;    /* 호환 기능 플래그 */
    __u32   s_feature_incompat;  /* 비호환 기능 플래그 */
    __u32   s_feature_ro_compat; /* 읽기 전용 호환 기능 플래그 */
    __u8    s_uuid[16];          /* 128비트 파일 시스템 UUID */
    char    s_volume_name[16];   /* 볼륨 이름 */
    char    s_last_mounted[64];  /* 마지막 마운트 지점 */
    __u32   s_algorithm_usage_bitmap; /* 압축 알고리즘 비트맵 */
    
    /* 성능 힌트 */
    __u8    s_prealloc_blocks;   /* 파일용 미리 할당할 블록 수 */
    __u8    s_prealloc_dir_blocks; /* 디렉토리용 미리 할당할 블록 수 */
    __u16   s_padding1;          /* 패딩 */
    
    /* 저널링 지원 */
    __u8    s_journal_uuid[16];  /* 저널 UUID */
    __u32   s_journal_inum;      /* 저널 아이노드 번호 */
    __u32   s_journal_dev;       /* 저널 디바이스 번호 */
    __u32   s_last_orphan;       /* 고아 아이노드 리스트 헤드 */
    
    /* 디렉토리 인덱싱 지원 */
    __u32   s_hash_seed[4];      /* HTREE 해시 시드 */
    __u8    s_def_hash_version;  /* 기본 해시 버전 */
    __u8    s_reserved_char_pad; /* 패딩 */
    __u16   s_reserved_word_pad; /* 패딩 */
    
    /* 기타 옵션 */
    __u32   s_default_mount_opts; /* 기본 마운트 옵션 */
    __u32   s_first_meta_bg;     /* 첫 번째 메타데이터 블록 그룹 */
    __u32   s_reserved[190];     /* 예약 공간 */
} __attribute__((packed));

// inode 구조체
struct my_ext2_inode {
    __u16   i_mode;                 /* 파일 타입 및 접근 권한 */
    __u16   i_uid;                  /* 소유자 UID */
    __u32   i_size;                 /* 파일 크기 (바이트 단위) */
    __u32   i_atime;                /* 마지막 접근 시간 */
    __u32   i_ctime;                /* 생성 시간 */
    __u32   i_mtime;                /* 마지막 수정 시간 */
    __u32   i_dtime;                /* 삭제 시간 */
    __u16   i_gid;                  /* 그룹 ID */
    __u16   i_links_count;          /* 하드 링크 수 */
    __u32   i_blocks;               /* 할당된 블록 수 (512바이트 단위) */
    __u32   i_flags;                /* 파일 플래그 */
    __u32   i_osd1;                 /* OS 의존적 값 1 */
    __u32   i_block[15];            /* 데이터 블록 포인터 */
    __u32   i_generation;           /* 파일 버전 (NFS용) */
    __u32   i_file_acl;             /* 파일 ACL */
    __u32   i_dir_acl;              /* 디렉토리 ACL */
    __u32   i_faddr;                /* 단편화 주소 */
    __u8    i_osd2[12];             /* OS 의존적 값 2 */
} __attribute__((packed));

// 디렉토리 엔트리
struct my_ext2_dir_entry_2 {
    __u32   inode;
    __u16   rec_len;
    __u8    name_len;
    __u8    file_type;
    char    name[];  // 가변 길이 배열
} __attribute__((packed));

// 블록 그룹 디스크립터
struct my_ext2_group_desc {
    __u32   bg_block_bitmap;
    __u32   bg_inode_bitmap;
    __u32   bg_inode_table;
    __u16   bg_free_blocks_count;
    __u16   bg_free_inodes_count;
    __u16   bg_used_dirs_count;
    __u16   bg_pad;
    __u32   bg_reserved[3];
} __attribute__((packed));

#endif