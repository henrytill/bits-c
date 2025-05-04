#include <stdlib.h>

#include <curl/curl.h>

int main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);

	printf("curl version: %s\n", data->version);

	return EXIT_SUCCESS;
}
