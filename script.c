#include <windows.h>
#include <wininet.h>
#include <stdio.h>

const char *_connect()
{
    HINTERNET hOpen = 0;
    HINTERNET hConnect = 0;
    HINTERNET hRequest = 0;
    int remotePort = 443;
    const char *remoteHost = "a.b.c.d"; // Cannot disclose
    hOpen = InternetOpen("wininet-test", INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0);
    if (!hOpen) return "InternetOpen";
    hConnect = InternetConnect(hOpen, remoteHost, remotePort, 0, 0, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) return "InternetConnect";
    {
        DWORD flags =
            INTERNET_FLAG_CACHE_IF_NET_FAIL |
            INTERNET_FLAG_IGNORE_CERT_CN_INVALID |
            INTERNET_FLAG_IGNORE_CERT_DATE_INVALID |
            INTERNET_FLAG_KEEP_CONNECTION |
            INTERNET_FLAG_NO_CACHE_WRITE |
            INTERNET_FLAG_PRAGMA_NOCACHE |
            INTERNET_FLAG_RELOAD |
            INTERNET_FLAG_RESYNCHRONIZE |
            INTERNET_FLAG_SECURE;
        char url[100];
        sprintf(url, "http://%s:%d/", remoteHost, remotePort);
        hRequest = HttpOpenRequest(hConnect, "GET", "connect.html", "HTTP/1.0", url, 0, flags, 0);
        if (!hRequest) return "HttpOpenRequest";
    }
    {
        DWORD flags=0;
        DWORD bufferLength = sizeof(flags);
        if (!InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, &bufferLength)) {
            return "InternetQueryOption";
        }
        flags |= (SECURITY_FLAG_IGNORE_UNKNOWN_CA | SECURITY_FLAG_IGNORE_REVOCATION);
        if (!InternetSetOption(hRequest, INTERNET_OPTION_SECURITY_FLAGS, &flags, sizeof(flags))) {
            return "InternetSetOption";
        }
    }
    if (!HttpSendRequest(hRequest, 0, 0, 0, 0)) {
        return "HttpSendRequest";
    } else {
        char buffer[4];
        DWORD bufferSize = sizeof(buffer);
        if (!HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE, &buffer, &bufferSize, NULL)) {
        return "HttpQueryInfo";
        } else if (atoi(buffer) != 200) {
            return "status code";
        }
    }
    {
        INTERNET_DIAGNOSTIC_SOCKET_INFO idsi;
        DWORD bufferSize = sizeof(idsi);
        if (!InternetQueryOption(hRequest, INTERNET_OPTION_DIAGNOSTIC_SOCKET_INFO, &idsi, &bufferSize)) {
            return "InternetQueryOption";
        } else if (idsi.Socket == INVALID_SOCKET) {
            /* This is always the case on our Windows 7 platform, why? */
            return "invalid socket";
        }
    }
    return 0;
}

int main(int argc, const char **argv)
{
    const char *error = _connect();
    if (error) {
        printf("ERROR: %s (%d)\n", error, GetLastError());
    } else {
        printf("SUCCESS\n");
    }
    return 0;
}
