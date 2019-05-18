#include <curl/curl.h>

// compiling:
//  macOS c++ -std=c++11 main.cpp -lcurl
//
// taken from the tutorial:
//  https://curl.haxx.se/libcurl/c/url2file.html

void print_curl_info();

size_t write_function(void* ptr, size_t size, size_t nmemb, void* user_data)
{
    FILE* file = (FILE*)user_data;
    size_t written = fwrite(ptr, size, nmemb, file);
    return written;
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("USAGE: %s <url>\n", argv[0]);
        return 0;
    }

    CURL* curl = nullptr;
    CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
    if(ret != CURLE_OK)
    {
        printf("ERROR: curl_global_init returned %d\n", ret);
        return 1;
    }

    print_curl_info();

    curl = curl_easy_init();
    if(!curl)
    {
        printf("ERROR: curl_easy_init returned nullptr\n");
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, argv[1]);   // set the url
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);     // enable debug output
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_function);


    const char* outfilename = "page.html";
    FILE* outfile = fopen(outfilename, "wb");
    if(outfile)
    {
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);

        printf("\nDownloading from %s to %s\n\n", argv[1], outfilename);

        curl_easy_perform(curl); // execute the task
    }
    else
    {
        printf("ERROR: failed to open %s\n", outfilename);
    }

    // cleanup
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return 0;
}

void print_curl_info()
{
    curl_version_info_data* info = curl_version_info(CURLVERSION_NOW);

    printf("\nCURL Library Info\n\n");

    printf("curl version                      : %s\n", info->version);
    printf("curl_version_info_data struct age : %d\n", info->age);
    printf("ssl version                       : %s\n", info->ssl_version);
    printf("libz version                      : %s\n", info->libz_version);
    printf("libidn version                    : %s\n", info->libidn);
    printf("libssh version                    : %s\n", info->libssh_version);
    //printf("brotli version: %s\n", info->brotli_version); // only available in struct age 4 or higher
    
#define PRINT_FEATURE(f) printf("%-25s: %s\n", #f, (info->features & f ? "yes":"no"))
    PRINT_FEATURE(CURL_VERSION_IPV6);
    PRINT_FEATURE(CURL_VERSION_KERBEROS4);
    PRINT_FEATURE(CURL_VERSION_KERBEROS5);
    PRINT_FEATURE(CURL_VERSION_SSL);
    PRINT_FEATURE(CURL_VERSION_LIBZ);
    PRINT_FEATURE(CURL_VERSION_NTLM);
    PRINT_FEATURE(CURL_VERSION_GSSNEGOTIATE);
    PRINT_FEATURE(CURL_VERSION_DEBUG);
    PRINT_FEATURE(CURL_VERSION_CURLDEBUG);
    PRINT_FEATURE(CURL_VERSION_ASYNCHDNS);
    PRINT_FEATURE(CURL_VERSION_SPNEGO);
    PRINT_FEATURE(CURL_VERSION_LARGEFILE);
    PRINT_FEATURE(CURL_VERSION_IDN);
    PRINT_FEATURE(CURL_VERSION_SSPI);
    PRINT_FEATURE(CURL_VERSION_GSSAPI);
    PRINT_FEATURE(CURL_VERSION_CONV);
    PRINT_FEATURE(CURL_VERSION_TLSAUTH_SRP);
    PRINT_FEATURE(CURL_VERSION_NTLM_WB);
    PRINT_FEATURE(CURL_VERSION_HTTP2);
    PRINT_FEATURE(CURL_VERSION_UNIX_SOCKETS);
    PRINT_FEATURE(CURL_VERSION_PSL);
    PRINT_FEATURE(CURL_VERSION_HTTPS_PROXY);
    //PRINT_FEATURE(CURL_VERSION_MULTI_SSL); // added 7.56.0
    //PRINT_FEATURE(CURL_VERSION_BROTLI); // added 7.57.0
}
