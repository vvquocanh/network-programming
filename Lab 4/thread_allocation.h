typedef struct {
	pthread_t tid;
	int is_executing;
	
} my_thread;

int allocate_thread_map(my_thread* thread_map, unsigned int map_size) {
	thread_map = (my_thread *)malloc(sizeof(my_thread) * map_size);
	
	if (thread_map == NULL) return -1;
	
	for (int i = 0; i < map_size; i++) {
		my_thread new_thread;
		new_thread.is_executing = 0;
		thread_map[i] = new_thread;
	}
	
	return 1;
}

pthread_t allocate_thread(my_thread* thread_map, unsigned int map_size) {
	if (thread_map == NULL) return -1;
	
	for (int i = 0; i < map_size; i++) {
		if (thread_map[i].is_executing == 1) continue;
		
		thread_map[i].is_executing = 1;
		return thread_map[i].tid; 
	}
	
	return -1;
}

void release_thread(my_thread* thread_map, unsigned int map_size, pthread_t tid) {
	for (int i = 0; i < map_size; i++) {
		if (thread_map[i].tid != tid) continue;
		
		thread_map[i].is_executing = 0;
		break;
	}
}
