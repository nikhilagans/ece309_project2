#include "core/sentinel_scanner.h"


    SentinelScanner::SentinelScanner(std::string sentinel){
        sentinel_ = sentinel;
        pending_ = "";
    }

    // Feed the next chunk. Returns text guaranteed NOT to be part of
    // the sentinel (safe to print immediately) and whether the
    // sentinel has now been fully seen.
    SentinelScanner::Out SentinelScanner::feed(std::string_view chunk){
        std::string new_text = pending_;
            new_text += chunk;

            if(std::size_t s_index = new_text.find(sentinel_); s_index != std::string::npos){
                SentinelScanner::Out result;
                result.safe_text = new_text.substr(0,s_index);
                result.sentinel_found = true;
                pending_.clear();
                return result;
            }
            else if (std::size_t f_index = new_text.find(sentinel_[0]);
                f_index != std::string::npos) {
                std::size_t size = new_text.size();

                    while (f_index < size) {
                        std::string test = new_text.substr(f_index);

                            if (test.size() < sentinel_.size() &&
                            test.compare(sentinel_.substr(0, test.size())) == 0) {
                                pending_ = test;
                                SentinelScanner::Out result;
                                result.safe_text = new_text.substr(0, f_index);
                                result.sentinel_found = false;
                                return result;
                            }

                            f_index = new_text.find(sentinel_[0], f_index + 1);
                    }
            }       

                            SentinelScanner::Out result;
                            result.safe_text = new_text;
                            result.sentinel_found = false;
                            pending_.clear();
                            return result;
}


    // Call once, after the stream ends, to release any text still
    // being held back.
    SentinelScanner::Out SentinelScanner::flush(){
        SentinelScanner::Out result;
        result.safe_text = pending_;
        result.sentinel_found = false;
        pending_.clear();   
        return result;
    }
