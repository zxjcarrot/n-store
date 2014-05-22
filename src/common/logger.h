#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdlib.h>
#include <unistd.h>

#include <mutex>
#include <vector>
#include <sstream>
#include <string>

#include "record.h"

using namespace std;

// LOGGING

class entry{
    public:
        entry(txn _txn, record* _before_image, record* _after_image) :
            transaction(_txn),
            before_image(_before_image),
            after_image(_after_image){}

        //private:
        txn transaction;
        record* before_image;
        record* after_image;
};


class logger {
    public:
		logger() {
			std::string log_file_name = "";
			log_file = NULL;
			log_file_fd = -1;
		}

        void set_path(std::string name, std::string mode){
            std::string log_file_name = name;

            log_file = fopen(log_file_name .c_str(), mode.c_str());
            if (log_file != NULL) {
                //cout << "Log file: " <<log_file_name<< endl;
            }

            log_file_fd = fileno(log_file);
        }

        void push(entry e){
        	std::lock_guard<std::mutex> lock(log_access);

            log_queue.push_back(e);
        }

        int write(){
            int ret ;
            stringstream buffer_stream;
            string buffer;

            {
				std::lock_guard<std::mutex> lock(log_access);

				for (std::vector<entry>::iterator it = log_queue.begin(); it != log_queue.end(); ++it) {
					if ((*it).transaction.txn_type != "")
						buffer_stream << (*it).transaction.txn_type;

					if ((*it).before_image != NULL)
						buffer_stream << *((*it).before_image);

					if ((*it).after_image != NULL)
						buffer_stream << *((*it).after_image);

					buffer_stream << endl;
				}

				buffer = buffer_stream.str();
				size_t buffer_size = buffer.size();

				ret = fwrite(buffer.c_str(), sizeof(char), buffer_size, log_file);
				ret = fsync(log_file_fd);

				// Set end time
				/*
				 for (std::vector<entry>::iterator it = log_queue.begin() ; it != log_queue.end(); ++it){
				 	 (*it).transaction.end = std::chrono::system_clock::now();
				 	 std::chrono::duration<double> elapsed_seconds = (*it).transaction.end - (*it).transaction.start;
				 	 cout<<"Duration: "<< elapsed_seconds.count()<<endl;
				 }
				 */

				// Clear queue
				log_queue.clear();
            }

            return ret;
        }

        void close(){
        	std::lock_guard<std::mutex> lock(log_access);

        	fclose(log_file);
        }

    //private:
        FILE* log_file ;
        int log_file_fd;

        std::mutex log_access;
        vector<entry> log_queue;
};


#endif
