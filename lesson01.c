/* Напишите программу для управления телефонной книгой

Программа должна быть консольной, формат взаимодействия -- пользовательская команда, за которой следует ответ.

Язык программирования -- чистый Си.

Команды являются регистрозависимыми, перед/после и между аргументами команд могут идти пробельные символы в любом количестве. Команда всегда заканчивается нажатием на клавишу Enter.

УРОВЕНЬ 1 -- ОГРАНИЧЕНИЯ И КОМАНДЫ

считается, что на уровне 1 кол-во записей ограничено числом 1000

считается, что ни имя, ни телефон не могут быть длиннее 20 символов

help
	Выводит список команд

exit / quit / bye
	Выход из программы

add имя телефон
	Имя -- последовательность латинских букв без пробелов
	Телефон -- последовательность из цифр и дефисов, начинается и заканчивается цифрой
	Пример: add tim 916-123-45-67
	При добавлении имя автоматически приводится к виду, когда первая буква и только она является заглавной
	Добавлять запись с той же парой (имя, телефон) запрещается, т.е. команда должна это отслеживать и ругаться

list
	Выводит содержимое книги, одна запись -- одна строка, сортировка по имени от A до Z

find имя
	Выводит все записи с данным именем

findnum номер
	Выводит все записи с данным номером, сортировка по имени A-Z

delrec имя номер
	Удаляет запись (имя, номер)


Программа должна проверять корректность ввода (синтаксис) и выдавать сообщение об ошибке, если что-то не так

Программа не должна падать

УРОВЕНЬ 2 -- ОГРАНИЧЕНИЯ И КОМАНДЫ

ограничение на количество записей отсутствует

ограничения на длины имени и телефона отсутствуют

save файл
	Сохраняет содержимое книги в указанный файл

load файл
	Загружает содержимое из файла, в случае успешной загрузки текущее содержимое удаляется, если же произошла ошибка -- текущее содержимое остается неизменным

find должна уметь автоматически понимать, что ее просят найти -- имя или номер, т.е. надобность в команде findum отпадает

del имя/номер
	Ищет запись с данным именем или номером и удаляет ее. Если есть несколько таких записей, выводит их все с сортировкой по имени, нумеруя строки, и предлагает пользователю удалить конкретную запись по номеру строки

Возможность при запуске указать исполняемому файлу в качестве аргумента имя файла для первоначальной load-загрузки

УРОВЕНЬ 3

find начинает понимать символ ‘*’, который может подменять собой любую последовательность символов, т.е. становится возможным поиск по маске, например find ti* или find 916-301*

Возможность вводить не всю команду, а только ее уникальный префикс. т.е. Если других команд на ‘a’ нет, то a будет значить add. Если же есть неоднозначность, должно выводиться сообщение со списком команд с данным префиксом.

Реализовать возможность добавлять комментарии к записям (как необязательный аргумент к add и отдельной командой comment)

Сделать так, чтобы все работало, если имена содержат пробел */

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <errno.h>
// #include <assert.h>

// #define ANSI_BLACK_ON_WHITE "\x1b[30;47m"
// #define ANSI_YELLOW_ON_RED "\x1b[93;41m"
// #define ANSI_GREEN_ON_BLACK "\x1b[92;40m"
// #define ANSI_RESET "\x1b[0m"

#define ANSI_BLACK_ON_WHITE ""
#define ANSI_YELLOW_ON_RED ""
#define ANSI_GREEN_ON_BLACK ""
#define ANSI_RESET ""
#define MAX_NAME_LEN (32+1)
#define MAX_PHONE_LEN (13+1)
#define MAX_BOOK_SIZE 1000
#define MAX_PATH_LENGTH 80
#define CODE_HELP 1
#define CODE_ADD 2
#define CODE_LIST 3
#define CODE_FINDNAME 4
#define CODE_FINDNUM 5
#define CODE_SWAP 100
#define CODE_SORT 101
#define CODE_DELETE 127
#define CODE_EXIT 255


struct record_t
{
	char name[MAX_NAME_LEN];
	char phone[MAX_PHONE_LEN];
};

typedef struct record_t Record;

size_t LastRecord = 4;

static Record Records[MAX_BOOK_SIZE] =
{
	{ "Timothy", "915-204-33-42" },
	{ "Newbie", "432-212-53-32" },
	{ "Guy", "322-123-55-53"},
	{ "Annette", "923-126-20-24" }
};

char** strSplit( char* _str )
{
	char** result		= 0;
	size_t count		= 0;
	char* tmp			= _str;
	char* last_delimiter	= 0;
	char delim[3];
	delim[0]			= 32;
	delim[1]			= 9;
	delim[2]			= 0;

	while ( *tmp )
	{
		if ( delim[0] == *tmp || delim[1] == *tmp )
		{
			count++;
			last_delimiter = tmp;
		}

		tmp++;
	}

	count += last_delimiter < ( _str + strlen( _str ) - 1 );
	count++;
	result = ( char** )malloc( sizeof( char* ) * count );

	if ( result )
	{
		size_t idx = 0;
		char* token = strtok( _str, delim );

		while ( token )
		{
			*( result + idx++ ) = strdup( token );
			token = strtok( 0, delim );
			// printf( "\"%s\"\n", token );
		}

		*( result + idx ) = 0;
	}

	return result;
}

static const char* cmdStrings[256][3] =
{
	[CODE_HELP] = { "help" },
	[CODE_EXIT] = { "exit", "bye", "quit" },
	[CODE_ADD] = { "add" },
	[CODE_LIST] = { "list" },
	[CODE_FINDNAME] = { "find" },
	[CODE_FINDNUM] = { "findnum" },
	[CODE_SWAP] = { "swap" },
	[CODE_SORT] = { "sort" },
	[CODE_DELETE] = { "del", "delete" },
};

int codeFromCommand( const char* command )
{
	for ( size_t i = 0; i < 256; ++i )
	{
		for ( size_t j = 0; j < 3; ++j )
		{
			if ( cmdStrings[i][j] && !strcmp( command, cmdStrings[i][j] ) ) return i;
		}
	}

	return 0;
}

void displayHelp()
{
	printf( "%sAvailable commands listing:%s\n\t", ANSI_YELLOW_ON_RED, ANSI_RESET );

	for ( size_t i = 0; i < 256; ++i )
	{
		for ( size_t j = 0; j < 3; ++j )
		{
			if ( cmdStrings[i][j] )
			{
				printf( "\"%s%s%s\" ", ANSI_GREEN_ON_BLACK , cmdStrings[i][j], ANSI_RESET );
			}
		}
	}

	printf( "\n" );
}

int addRecord( const char _name[], const char _phone[], size_t _lastrecord )
{
	if ( lookUpName( _name ) >= 0 && lookUpPhone( _phone ) >= 0 )
	{
		printf( "This Name and Number record already exists.\n" );
		return ( int )_lastrecord;
	};

	strcpy( Records[_lastrecord].name, _name );

	strcpy( Records[_lastrecord].phone, _phone );

	return ( int )++_lastrecord;
}

void swap( size_t _left, size_t _right )
{
	if ( _left == _right )
	{
		printf( "Cannot swap the same record.\n" );
		return;
	}

	if ( _left >= LastRecord || _right >= LastRecord )
	{
		printf( "One or both records index is out of book range.\n" );
		return;
	}

	char name[MAX_NAME_LEN];
	char phone[MAX_PHONE_LEN];
	strcpy( name, Records[_left].name );
	strcpy( phone, Records[_left].phone );
	strcpy( Records[_left].name, Records[_right].name );
	strcpy( Records[_left].phone, Records[_right].phone );
	strcpy( Records[_right].name, name );
	strcpy( Records[_right].phone, phone );
}

void sortArray()
{
	for ( size_t i = 0; i < LastRecord; ++i )
	{
		for ( size_t j = i + 1; j < LastRecord; ++j )
		{
			if ( strcmp( Records[i].name, Records[j].name ) < 0 )
			{
				continue;
			}
			else if ( strcmp( Records[i].name, Records[j].name ) == 0 )
			{
				continue;
			}
			else
			{
				swap( i, j );
			}
		}
	}
}

bool isPhoneValid( const char* _phone )
{
	if ( strlen( _phone ) != MAX_PHONE_LEN - 1 )
	{
		return false;
	}

	return ( isdigit( *_phone ) &&
	         isdigit( *( _phone + 1 ) ) &&
	         isdigit( *( _phone + 2 ) ) &&
	         *( _phone + 3 ) == '-' &&
	         isdigit( *( _phone + 4 ) ) &&
	         isdigit( *( _phone + 5 ) ) &&
	         isdigit( *( _phone + 6 ) ) &&
	         *( _phone + 7 ) == '-' &&
	         isdigit( *( _phone + 8 ) ) &&
	         isdigit( *( _phone + 9 ) ) &&
	         *( _phone + 10 ) == '-' &&
	         isdigit( *( _phone + 11 ) ) &&
	         isdigit( *( _phone + 12 ) ) );
}

bool isNumeric( const char _token[] )
{
	bool retValue = 1;

	for ( size_t i = 0; i < strlen( _token ); ++i )
	{
		retValue = ( retValue && isdigit( _token[i] ) );
	}

	return retValue;
}

char* validateName( const char* _name )
{
	char* result = malloc( sizeof( char* ) * ( strlen( _name ) + 1 ) );
	strcpy( result, _name );
	*result = toupper( *_name );

	for ( size_t i = 1; i < strlen( _name ); ++i )
	{
		*( result + i ) = tolower( *( _name + i ) );
	}

	return result;
}

void listBook()
{
	for ( size_t i = 0; i < LastRecord; ++i )
	{
		printf( "%ld : %s : %s\n", i, Records[i].name, Records[i].phone );
	}
}

/* Хотелось бы сделать возвращение из функции множества значений;
делать в функции просто printf как-то грубо, к тому же проверки
на наличие записи в базе сломаются. */

int lookUpName( const char _name[] )
{
	for ( size_t i = 0; i < MAX_BOOK_SIZE; ++i )
	{
		if ( !strcmp( Records[i].name, _name ) )
		{
			return ( int )i;
		}
	}

	return -1;
}

int lookUpPhone( const char _phone[] )
{
	for ( size_t i = 0; i < MAX_BOOK_SIZE; ++i )
	{
		if ( strcmp( Records[i].phone, _phone ) == 0 )
		{
			return ( int )i;
		}
	}

	return -1;
}


bool deleteRecord( size_t _delete )
{
	const char* _empty = "";

	if ( _delete >= LastRecord )
	{
		printf( "Unknown index\n" );
		return false;
	}

	strcpy( Records[_delete].name, _empty );
	strcpy( Records[_delete].phone, _empty );
	swap( _delete, LastRecord - 1 );
	--LastRecord;
	return true;
}

int main()
{
	bool exitFlag = 0;
	// time_t mtime = time( NULL );
	char path[MAX_PATH_LENGTH];
	getcwd( path, MAX_PATH_LENGTH );
	printf( "Current Directory = %s\n", path );
	FILE* log = fopen( "log.txt", "a+" );

	if ( log == NULL )
	{
		printf( "Error opening \"log.txt\" file (errno = %d).", errno );
	}

	while ( !exitFlag )
	{
		char str[100] = { 0 };
		char** tokens = NULL;
		printf( "> " );
		gets( str );
		// char* timeStr = ctime( &mtime );
		// timeStr[strlen( timeStr ) - 1] = '\0';
		fprintf( log, "%s\n", str );
		fflush( log );

		if ( strlen( str ) )
		{
			tokens = strSplit( str );
			// printf( "\"%s\" - \"%d\"- \"%d\"\n", tokens[0], tokens[1], tokens[2] );
			// если введена одна команда, то 1 и 2 указывают в никуда (не возвращаются из)
			// функции разделения строки, и обращение к ним есть UBs

			switch ( codeFromCommand ( tokens[0] ) )
			{
			case CODE_HELP:
				displayHelp();
				break;

			case CODE_ADD:
				if ( tokens[1] && isPhoneValid( tokens[2] ) )
				{
					LastRecord = addRecord( validateName( tokens[1] ), tokens[2], LastRecord );
					sortArray();
				}
				else
				{
					printf( "No name or valid phone provided.\n" );
				};

				break;

			case CODE_LIST:
				listBook();
				break;

			case CODE_FINDNAME:
				printf( "Record ID with name %s is %d\n", tokens[1], lookUpName( tokens[1] ) );
				break;

			case CODE_FINDNUM:
				printf( "Record ID with phone %s is %d\n", tokens[1], lookUpPhone( tokens[1] ) );
				break;

			case CODE_SWAP:
				if ( tokens[1] && isNumeric(tokens[1]) && tokens[2] && isNumeric(tokens[2]) )
				{
					swap( atoi( tokens[1] ), atoi( tokens[2] ) );
				}
				else
				{
					printf("Not valid input. Command syntax: \"swap <integer> <integer>\".\n");
				}
				break;

			case CODE_SORT:
				sortArray();
				break;

			case CODE_DELETE:
				if ( tokens[1] && isNumeric( tokens[1] ) && atoi( tokens[1] ) < LastRecord )
				{
					deleteRecord( atoi( tokens[1] ) );
					sortArray();
				}
				else
				{
					printf( "No valid record number entered.\n" );
				}

				break;

			case CODE_EXIT:
				exitFlag = true;
				break;

			default:
				printf( "Unknown command entered. Enter \"%shelp%s\" for a list of available commands.\n", ANSI_GREEN_ON_BLACK, ANSI_RESET );
			}
		}
		else
		{
			printf( "Empty string!\n" );
		}
	}

	fprintf( log, "\n" );
	fclose( log );
	return 0;
}
