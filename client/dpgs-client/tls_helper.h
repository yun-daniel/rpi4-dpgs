#ifndef TLS_HELPER_H
#define TLS_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _GTlsDatabase GTlsDatabase;

/**
 *  \brief CA 인증서 파일 경로로부터 GTlsFileDatabase를 만들어 반환합니다.
 *  \param ca_path   CA 인증서(.crt) 파일의 절대/상대 경로
 *  \return 생성된 GTlsDatabase* 또는 실패 시 NULL
 */
GTlsDatabase* load_tls_database(const char *ca_path);

#ifdef __cplusplus
}
#endif

#endif // TLS_HELPER_H
