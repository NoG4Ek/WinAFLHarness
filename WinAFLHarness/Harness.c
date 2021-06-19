#define _CRT_SECURE_NO_WARNINGS

#include "Harness.h"
#include "getopt.h"

#define DEBUG_LOG 0

#if DEBUG_LOG != 1
    #define PRINTF(...) printf(__VA_ARGS__)
#else
    #define PRINTF(...)
#endif

static const char const* output_filename = "out.txt";

void redirect_all_output_to_file(char const* filename)
{
	freopen(filename, "w", stdout);
	freopen(filename, "w", stderr);
}

void restore_all_output_to_con()
{
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
}

char const* read_stacktrace_from_file(char const* filename)
{
		FILE* stacktrace_file = fopen(filename, "rb");

		fseek(stacktrace_file, 0, SEEK_END);
		long stacktrace_size = ftell(stacktrace_file);
		fseek(stacktrace_file, 0, SEEK_SET);

		char* exc_text = malloc(sizeof(char) * (stacktrace_size + 1));

		long rd = fread(exc_text, sizeof(char), stacktrace_size, stacktrace_file);

		exc_text[rd] = '\0';

		fclose(stacktrace_file);

		return exc_text;
}

__declspec(dllexport) __declspec(noinline) int loadDLLFunc(int argc, char** argv,
    char const* library_name, char const* func_name,
    boolean should_redirect_output)
{
	int status = 0;
	HINSTANCE testLibrary = LoadLibrary(library_name);
	if (testLibrary)
	{
		PRINTF("%s library was found\n", library_name);
        TestFunc _TestFunc = (TestFunc) GetProcAddress(testLibrary, func_name);
		if (_TestFunc)
		{
			PRINTF("%s function was found\n", func_name);

            if (should_redirect_output) redirect_all_output_to_file(output_filename);
			status = _TestFunc(argc, argv);
			if (should_redirect_output) restore_all_output_to_con();
		}
		FreeLibrary(testLibrary);
	}
	return status;
}

__declspec(dllexport) __declspec(noinline) int main(int argc, char** argv)
{
	char* library_name = "";
	char* func_name = "";
	char* rsi = "";
	char* rse = "";
	char* path_to_input = "";

	int c = 0;
	while ((c = getopt(argc, argv, "l:f:i:e:p:")) != -1)
	{
		switch (c)
		{
		case (int)('l'):
			library_name = optarg;
			break;

		case (int)('f'):
			func_name = optarg;
			break;

		case (int)('i'):
			rsi = optarg;
			break;

		case (int)('e'):
			rse = optarg;
			break;

		case (int)('p'):
			path_to_input = optarg;
			break;

		case (int)('?'):
			{
				char* msg;
				msg = "Illegal option\nUsage: -l -f -i -e -p";
				PRINTF("%s\n", msg);
			}
			return FALSE;
			break;

		default:
		{
			PRINTF("No handler for option\n");
		}
			return FALSE;
			break;
		}
	}
	
    // DONE: switch to normal argument handling via getopt or smth
	int status_main_fuzz_function = 0;
	int status_exc_sort_function = 0;

	char* args_for_fuzz_function[] = { "programUnderFuzz", path_to_input };

	status_main_fuzz_function = loadDLLFunc(2, args_for_fuzz_function, library_name, func_name, TRUE);

	PRINTF("%u status fuzz function\n", status_main_fuzz_function);

	if (status_main_fuzz_function == 1) {

		char* exc_text = read_stacktrace_from_file(output_filename);

		char* args_for_sort_exc[] = { "sortExc", exc_text, rsi, rse };

		PRINTF("TEXT:\n%s\n", exc_text);
		PRINTF("RSI:\n%s\n", rsi);
		PRINTF("RSE:\n%s\n", rse);

		status_exc_sort_function = loadDLLFunc(4, args_for_sort_exc, "SortExceptions.dll", "run_main", FALSE);

		PRINTF("%u status sort exception function\n", status_exc_sort_function);

		if (status_exc_sort_function == 1) {
		    // TODO: Move filename to yet another argument

			FILE* exc_seen_file = fopen("newExceptions.txt", "at");
			
			fprintf(exc_seen_file, "%s\n", exc_text);
		
			fclose(exc_seen_file);
		}

		free(exc_text);

        if (status_exc_sort_function == 1) {
            PRINTF("CRASH\n");
            int e = 1 / (status_exc_sort_function - 1);
            e++;
//            int* arr = NULL;
//            int ee = arr[0] / arr[0];
//            main(arr[0], arr[0]);
            return e;
        }
	}
}
