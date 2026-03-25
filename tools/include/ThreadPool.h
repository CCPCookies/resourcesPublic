#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

template <class... ArgumentTypes>
class ThreadPool
{
public:

	ThreadPool(size_t numberOfThreads = std::thread::hardware_concurrency()):
		m_maxNumberOfThreads(numberOfThreads),
		m_numberOfActiveThreads(0),
		m_stop(false),
		m_numTasks(0)
	{

	}

	~ThreadPool()
	{
		m_stop = true;

		m_conditional.notify_all();

		while (IsRunning()) {}

		for (std::thread* t : m_threads)
		{
			t->join();
			delete t;
		}
	}

	void Start( )
	{
		// Create threads
		for (size_t i = 0; i < m_maxNumberOfThreads; ++i)
		{
			m_threads.push_back(new std::thread(&ThreadPool::Worker, this));

			m_numberOfActiveThreads++;
		}
	}

    int GetNumberOfPendingTasks()
    {
		return m_numTasks;
    }

	void Join()
	{
        if (m_maxNumberOfThreads == 0)
        {
            while (!m_tasks.empty())
            {
				std::unique_ptr<Task> task = std::move( m_tasks.front() );

				m_tasks.pop();

                std::apply( [task = std::move( task )]( auto&&... args ) { task->function( args... ); }, task->arguments );

				m_numTasks--;
            } 
        }
        else
        {
			std::unique_lock<std::mutex> lock( m_waitMutex );

			m_joinConditional.wait( lock, [this] {
				return m_numTasks == 0;
			} );
        }
		
	}

	void AddTask( std::function<void( ArgumentTypes... values )> function, ArgumentTypes... arguments )
	{
		std::unique_ptr<Task> task = std::make_unique<Task>();

		task->arguments = std::make_tuple(arguments...);

		task->function = function;
			
		std::unique_lock<std::mutex> lock(m_queueMutex);

		m_tasks.push(std::move(task));

		m_numTasks++;

		if (IsRunning())
		{
			m_conditional.notify_one();
		}
	}

private:

	struct Task
	{
		std::tuple<ArgumentTypes...> arguments;
		std::function<void( ArgumentTypes... values )> function;
	};

	void Worker()
	{
		while (true)
		{
			std::unique_ptr<Task> task;

			{
				std::unique_lock<std::mutex> lock(m_queueMutex);

				m_conditional.wait(lock, [this] {
					return !m_tasks.empty() || m_stop == true;
					});


				if (m_stop)
				{
					// Stop thread
					m_numberOfActiveThreads--;
					return;
				}

				task = std::move(m_tasks.front());

				m_tasks.pop();
			}

			std::apply( [task = std::move( task )]( auto&&... args ) { task->function( args... ); }, task->arguments );

			m_numTasks--;

            m_joinConditional.notify_all();
		}
	}

	bool NumberOfTasksPending() const
	{
		return m_numTasks > 0;
	}

	bool IsRunning() const
	{
		return m_numberOfActiveThreads > 0;
	}

private:

	size_t m_maxNumberOfThreads;

	std::atomic<size_t> m_numberOfActiveThreads;

	std::vector<std::thread*> m_threads;

	std::mutex m_queueMutex;

	std::queue<std::unique_ptr<Task>> m_tasks;

	std::atomic<int> m_numTasks;

	std::condition_variable m_conditional;

    std::mutex m_waitMutex;

    std::condition_variable m_joinConditional;

	bool m_stop;

};
