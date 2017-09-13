#ifndef _REDISCPP_H_
#define _REDISCPP_H_

#include <hiredis/hiredis.h>
#include <hiredis/async.h>

#include <string>
#include <vector>
#include <string.h>

using namespace std;

#if (defined(_WIN32) && _MSC_VER >= 1800) || __cplusplus >= 201103L
	#include <unordered_map>
	typedef unordered_map<string, string> RedisMapType;
#else
	#include <map>
	typedef map<string, string> RedisMapType;
#endif

typedef vector<string> RedisArrayType;


#define RedisKey const char *
#define RedisField const char *

#if defined(_WIN32)

#if defined(REDIS_EXPORT_API)

#define EXPORT_API __declspec(dllexport)

#else

#define EXPORT_API __declspec(dllimport)


#endif

#else

#define EXPORT_API

#endif

class RedisReplyObject;
class EXPORT_API RedisClient
{
public:
	RedisClient();
	~RedisClient();
public:
	bool Connect(const char *ip, int port, int timeoutsec);

	bool Connect(const char *ip, int port, int db, const char *password, int timeoutsec);

public:
	string Get(RedisKey key);

	bool Set(RedisKey key, const char *value);

	bool Del(RedisKey key);

	string HGet(RedisKey key, RedisField field);

	bool Expire(RedisKey key, int seconds);

	bool ExpireAt(RedisKey key, time_t timestamp);

	bool Persist(RedisKey key);

	bool PExpire(RedisKey key, int milliseconds);

	bool PExpireAt(RedisKey key, time_t  timestamp_ms);

	////////////////////////////
	RedisMapType HGetall(RedisKey key);

	bool HDel(RedisKey key, RedisField field);

	int HIncrby(RedisKey key, RedisField field, PORT_LONGLONG value);

	bool HExists(RedisKey key, RedisField field);

	int HLen(RedisKey key);

	RedisArrayType HKeys(RedisKey key);

	RedisArrayType LRange(RedisKey key, int beg, int end);

	RedisArrayType LRangeall(RedisKey key);

	bool Exists(RedisKey key);

	

	RedisArrayType MGet(RedisKey keys);

private:
	bool CheckReplyStatus(const redisReply *reply);

private:
	redisContext *m_pCtx;
};

class RedisReplyObject
{
public:
	RedisReplyObject(redisReply *reply)
		:m_reply(reply)
	{}

	~RedisReplyObject()
	{
		if (m_reply)
		{
			freeReplyObject(m_reply);
		}
	}

	redisReply *operator->() { return m_reply; }

	bool operator == (const int v) { return m_reply == (redisReply *)v; }

	const redisReply * GetPrt() { return m_reply; }

private:
	RedisReplyObject & operator=(const RedisReplyObject &) {}

	RedisReplyObject(const RedisReplyObject &) {}

private:
	 redisReply *m_reply;
};

#endif
