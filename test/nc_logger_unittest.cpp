#include "stdafx.h"
#include "ncserver.h"
#include "gtest.h"
#include "nc_log.h"

using namespace ncserver;

class NcLogTest : public ::testing::Test, public NcLogDelegate
{
public:
	static void SetUpTestCase()
	{
		m_nclog = &NcLog::instance();
		m_nclog->registerUpdateLogLevelSignal();
		m_nclog->init("echo", LogLevel_info);
	}

	static void TearDownTestCase()
	{
	}

	void SetUp()
	{
		m_nclog->setDelegate(this);
		m_lastMessage = copyStr("", 0);
		m_lastRawMessage = copyStr("", 0);
		m_lastLogLevel = copyStr("none", 4);
	}

	void TearDown()
	{
		free(m_lastMessage);
		free(m_lastLogLevel);
	}

	const char* lastMessage() { return m_lastMessage; }
	const char* lastRawMessage() { return m_lastRawMessage; }
	const char* lastLogLevel() { return m_lastLogLevel; }

	virtual void nclogWillOutputMessage(bool hasHeader, const char* message)
	{
		const char* text = NULL;
		if (hasHeader)
		{
			text = strchr(message, ']') + 2; // skip file, lineno, func name
			const char* logLevel = strchr(message, ':') + 2;
			const char* endOfLogLevel = strchr(logLevel, ':');
			m_lastLogLevel = copyStr(logLevel, endOfLogLevel - logLevel);
		}
		else
		{
			text = message;
			m_lastLogLevel = copyStr("none", 4);
		}

		free(m_lastMessage);
		free(m_lastRawMessage); 
		m_lastMessage = copyStr(text, strlen(text));
		m_lastRawMessage = copyStr(message, strlen(message));
	}

protected:
	char* m_lastMessage;
	char* m_lastRawMessage;
	char* m_lastLogLevel;
	char* m_file;
	char* m_level;
	char* m_function;
	static NcLog* m_nclog;

	char* copyStr(const char* str, size_t len)
	{
		char* newCopy = (char*)malloc(len + 1);
		memcpy(newCopy, str, len);
		newCopy[len] = '\0';
		return newCopy;
	}

	void parsingModuleName(const char* rawMessage)
	{
		const char* p1 = strchr(rawMessage, '(');
		long long len1 = p1 - rawMessage;
		m_file = copyStr(rawMessage, len1);

		const char* p2 = strchr(p1, ':');
		const char* p3 = strchr(p2 + 1, ':');
		long long len2 = p3 - p2 - 2;
		m_level = copyStr(p2 + 2, len2);

		const char* p4 = strchr(p3, '[');
		const char* p5 = strchr(p4 + 1, ']');
		long long len3 = p5 - p4 - 1;
		m_function = copyStr(p3 + 3, len3);
	}

	void freeModuleName()
	{
		free(m_file);
		free(m_level);
		free(m_function);
	}
};

NcLog* NcLogTest::m_nclog = NULL;

TEST_F(NcLogTest, basic)
{
	ASYNC_LOG_ALERT("Hello %s", "world");
	EXPECT_STREQ(lastMessage(), "Hello world");
}

TEST_F(NcLogTest, header)
{
	ASYNC_LOG_ALERT("Hello %s", "world");
	parsingModuleName(lastRawMessage());
	EXPECT_TRUE(strstr(m_file, "nc_logger_unittest.cpp") != NULL);
	EXPECT_STREQ(m_level, m_lastLogLevel);
	EXPECT_TRUE(strstr(m_function, "TestBody") != NULL);
	freeModuleName();
}

TEST_F(NcLogTest, zeroParam)
{
	ASYNC_LOG_ALERT("Hello world");
	EXPECT_STREQ(lastMessage(), "Hello world");
}

TEST_F(NcLogTest, raw)
{
	ASYNC_RAW_LOG("Hello world");
	EXPECT_STREQ(lastMessage(), "Hello world");

	ASYNC_RAW_LOG("Hello %s", "cplusplus");
	EXPECT_STREQ(lastMessage(), "Hello cplusplus");
}

TEST_F(NcLogTest, 10k)
{
	// 10k*'a' = 'aaaaaaaaaaaaaaaaaaaaaaaa....'
	char largeBuffer[1024 * 10];
	memset(largeBuffer, 'a', sizeof(largeBuffer));
	largeBuffer[sizeof(largeBuffer) - 1] = 0;
	ASYNC_LOG_ALERT("%s-%d",largeBuffer, 99);
	EXPECT_TRUE(lastMessage()[0] == 'a');
	EXPECT_TRUE(lastMessage()[sizeof(largeBuffer) - 2] == 'a');
	EXPECT_TRUE(lastMessage()[sizeof(largeBuffer) + 1] == '9');
	EXPECT_TRUE(lastMessage()[sizeof(largeBuffer)] == '9');
	EXPECT_EQ(strlen(lastMessage()), strlen(largeBuffer) + 3);
}

TEST_F(NcLogTest, 65k)
{
	// 65k*'a' = 'aaaaaaaaaaaaaaaaaaaaaaaa....'
	char largeBuffer[1024 * 65];
	memset(largeBuffer, 'a', sizeof(largeBuffer));
	largeBuffer[sizeof(largeBuffer) - 1] = 0;
	ASYNC_LOG_ALERT("%s-%d", largeBuffer, 99);
	EXPECT_EQ(strlen(lastMessage()), 0);
}

TEST_F(NcLogTest, logLevel)
{
	LogLevel originalLevel = NcLog::instance().logLevel();

 	NcLog::instance().setLogLevel(LogLevel_notice);
	EXPECT_EQ(NcLog::instance().logLevel(), LogLevel_notice);
	ASYNC_LOG_NOTICE("notice 1");
	EXPECT_STREQ(lastMessage(), "notice 1");
	EXPECT_STREQ(lastLogLevel(), LogLevel_toString(LogLevel_notice));

	NcLog::instance().setLogLevel(LogLevel_warning);
	EXPECT_EQ(NcLog::instance().logLevel(), LogLevel_warning);
	ASYNC_LOG_NOTICE("notice 2");
	ASYNC_LOG_INFO("info");
	EXPECT_STREQ(lastMessage(), "notice 1");
	EXPECT_STREQ(lastLogLevel(), LogLevel_toString(LogLevel_notice));

	ASYNC_RAW_LOG("raw");
	EXPECT_STREQ(lastMessage(), "raw");
	EXPECT_STREQ(lastLogLevel(), "none");

	NcLog::instance().setLogLevel(originalLevel);
}

#if !defined(LINUX)
TEST_F(NcLogTest, logPrintOnWindows)
{
	NcLog::instance().setDelegate(NULL);

	const char* fileName = __FILE__;
	const char* functionName = __FUNCTION__;

	int standardOut = _dup(1);
	FILE* logFile = freopen("abc.log", "w", stdout);
	ASYNC_LOG_NOTICE("This is a log that would be printed on stdout."); int line1 = __LINE__;
	ASYNC_LOG_INFO("And this log should be printed in a new line."); int line2 = __LINE__;
	fflush(logFile);
	_dup2(standardOut, 1);

	logFile = fopen("abc.log", "r");
	int start = ftell(logFile);
	fseek(logFile, 0, SEEK_END);
	int fileLen = ftell(logFile) - start;
	fseek(logFile, 0, SEEK_SET);
	EXPECT_GT(fileLen, 0);
	if (fileLen > 0)
	{
		char* actualLog = (char*)malloc(fileLen +1);
		memset(actualLog, 0, fileLen + 1);
		fread(actualLog, fileLen, 1, logFile);

		char* expectedLog = (char*)malloc(4096);
		memset(expectedLog, 0, 4096);
		_snprintf(expectedLog, 4096,
			"%s(%d): %s: [%s] This is a log that would be printed on stdout.\n"
			"%s(%d): %s: [%s] And this log should be printed in a new line.\n",
			fileName, line1, LogLevel_toString(LogLevel_notice), functionName,
			fileName, line2, LogLevel_toString(LogLevel_info), functionName);

		EXPECT_STREQ(actualLog, expectedLog);
		free(expectedLog);
		free(actualLog);
	}
	fclose(logFile);
	remove("abc.log");
}

#endif