#include "stdafx.h"
#include "ncserver/ncserver.h"
#include "gtest.h"

using namespace ncserver;

TEST(Request, basic)
{
	Request request;
	request.setQueryString("name=Chen%20Bowei&age=27&gender=male");
	EXPECT_STREQ(request.queryString(), "name=Chen Bowei&age=27&gender=male");
	EXPECT_STREQ(request.parameterForName("name"), "Chen Bowei");
	EXPECT_STREQ(request.parameterForName("age"), "27");
	EXPECT_STREQ(request.parameterForName("gender"), "male");
}
