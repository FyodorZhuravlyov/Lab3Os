#include <iostream>
#include <Windows.h>

struct thread {
	int array_size;
	int* arr;
	int thread_index;
	HANDLE stop_work;
	HANDLE start_work;
	HANDLE* terminate_or_continue;
};
CRITICAL_SECTION cs;

DWORD WINAPI thread_func(LPVOID params) {
	thread info = *((thread*)params);
	bool end_thread = false;
	int number_of_marked_elements = 0;
	srand(info.thread_index);

	WaitForSingleObject(info.start_work, INFINITE);

	while (!end_thread) {
		int ind = rand() % info.array_size;
		EnterCriticalSection(&cs);
		if (info.arr[ind] == 0) {
			Sleep(5);
			info.arr[ind] = info.thread_index;
			LeaveCriticalSection(&cs);
			number_of_marked_elements++;
			Sleep(5);
		}
		else {
			std::cout << "\nThread " << info.thread_index << ", number of marked elements " << number_of_marked_elements << ", can't mark element , index " << ind;
			LeaveCriticalSection(&cs);
			SetEvent(info.stop_work);
			int k = WaitForMultipleObjects(2, info.terminate_or_continue, FALSE, INFINITE) - WAIT_OBJECT_0;
			if (k == 0) {
				end_thread = true;
			}
		}
	}
	for (int i = 0; i < info.array_size; i++) {
		if (info.arr[i] == info.thread_index) { info.arr[i] = 0; }
	}
	return 0;
}


void Init_thread(HANDLE threads[], thread thread_info[], bool terminated_threads[], HANDLE start_work, HANDLE stopped_threads[], int array[], int size, int threads_number)
{
	for (int i = 0; i < threads_number; ++i) {
		thread_info[i].arr = array;
		thread_info[i].array_size = size;
		thread_info[i].thread_index = i + 1;
		thread_info[i].start_work = start_work;
		stopped_threads[i] = thread_info[i].stop_work = CreateEvent(NULL, TRUE, FALSE, NULL);
		thread_info[i].terminate_or_continue = new HANDLE[2];
		thread_info[i].terminate_or_continue[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
		thread_info[i].terminate_or_continue[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
		threads[i] = CreateThread(NULL, 0, thread_func, (LPVOID)(&thread_info[i]), NULL, NULL);
		terminated_threads[i] = false;
	}
}

int main() {
	InitializeCriticalSection(&cs);
	int size;
	std::cout << "Enter size of array ";
	std::cin >> size;
	int* array = new int[size] {};
	std::cout << "Enter number marker ";
	int threads_number;
	std::cin >> threads_number;

	HANDLE* threads = new HANDLE[threads_number];
	thread* thread_info = new thread[threads_number];
	bool* terminated_threads = new bool[threads_number];
	HANDLE start_work = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE* stopped_threads = new HANDLE[threads_number];


	Init_thread(threads, thread_info, terminated_threads, start_work, stopped_threads, array, size, threads_number);
	SetEvent(start_work);

	int ended_threads = 0;

	while (ended_threads != threads_number) {
		WaitForMultipleObjects(threads_number, stopped_threads, TRUE, INFINITE);
		std::cout << std::endl;
		for (int i = 0; i < size; i++)
		{
			std::cout << array[i] << " ";
		}
		std::cout << std::endl;
		bool is_thread_terminated = false;
		while (!is_thread_terminated) {
			int thread_to_terminate_ind;
			std::cout << "Enter index of thread  ";
			std::cin >> thread_to_terminate_ind;
			thread_to_terminate_ind--;
			if (thread_to_terminate_ind >= threads_number || thread_to_terminate_ind < 0) {
				std::cout << "No thread with such index\n";
				continue;
			}
			if (terminated_threads[thread_to_terminate_ind]) {
				std::cout << "Thread is already terminated\n";
			}
			else {
				SetEvent(thread_info[thread_to_terminate_ind].terminate_or_continue[0]);
				WaitForSingleObject(threads[thread_to_terminate_ind], INFINITE);
				//std::cout << std::endl;
				for (int i = 0; i < size; i++)
				{
					std::cout << array[i] << " ";
				}
				std::cout << std::endl;
				terminated_threads[thread_to_terminate_ind] = true;
				is_thread_terminated = true;
				ended_threads++;
			}
		}

		for (int j = 0; j < threads_number; ++j) {
			if (!terminated_threads[j]) {
				ResetEvent(thread_info[j].stop_work);
				SetEvent(thread_info[j].terminate_or_continue[1]);
			}
		}

	}

	std::cout << "All threads are terminated\n";

	CloseHandle(start_work);
	for (int i = 0; i < threads_number; ++i) {
		CloseHandle(threads[i]);
		CloseHandle(stopped_threads[i]);
		CloseHandle(thread_info[i].terminate_or_continue[0]);
		CloseHandle(thread_info[i].terminate_or_continue[1]);
	}
	DeleteCriticalSection(&cs);
	return 0;
}