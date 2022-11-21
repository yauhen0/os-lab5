#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class Channel
{
private:
	std::mutex mutex;
	int buffSize_;
	std::queue<T> queue;
	std::condition_variable variable;
	std::atomic<bool> closeChannel = false;

public:
	Channel(int buffSize) 
	{
		buffSize_ = buffSize;
	};

	void send(T&& data)
	{
		if (closeChannel)
			throw new std::runtime_error("err");
		

		std::unique_lock<std::mutex> locker(mutex);

		variable.wait(locker, [this] {
			return	queue.size() < buffSize_;
			});

		queue.push(data);
		variable.notify_one();
	}

	std::pair<T, bool> recv()
	{

		if (closeChannel && queue.empty())
			return std::make_pair(T(), false);
		

		std::unique_lock<std::mutex> locker(mutex);

		variable.wait(locker, [this] {
			return	!queue.empty();
			});

		T result = queue.front();
		queue.pop();
		variable.notify_one();
		return std::make_pair(result, true);
	}

	void close()
	{
		closeChannel.store(true);
		variable.notify_all();
	}


};