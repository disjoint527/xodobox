struct DataFiles
{
    struct Row
    {
        char code[10];
        uint32_t time;
        float last;
        double volume;
        double amount;
    };

    uint32_t num_rows = 0;
    uint32_t num_codes = 0;

    std::array<char [8], 8 * 1024> codes; /// sorted [600000,...
    std::array<uint32_t, 8 * 1024> latest; ///       [ 3300, 
	
	
    std::array<Row, 32 * 1024 * 1024> book;

    /// Row book[32 * 1024 * 1024]
    /// book.data() == &book[0]
    /// book.size() == 32 * 1024 * 1024;
    /// std::lower_bound
	
	static bool cmp(const char m[], const char target[])
	{
		int res = memcmp(m, target, 8); /// "SH600423" > "SH600000" return 1; res = -1, R = m; res = 1, L = m + 1;
		if(res > 0) return false;
		else return true;
	}
	
	int find_row(const char target_code[])
	{	
		auto lower = std::lower_bound(codes.begin(), &codes[num_codes], target_code, cmp);
		lower--;
		int cur = (int)(lower - codes.begin());
		std::string cur_str = std::string(codes[cur], 8);
		std::string target = std::string(target_code, 8);
		if(cur_str == target) return (lower - codes.begin());
		else return -1;
	}
	
    void add_row(const Row* array, size_t count)
    {	
    	if(num_rows==0)
    	{
			auto* start = codes.data();
    		for(size_t i = 0; i < count; i++)
    		{
				auto& dst = book[num_rows++];
				auto& src = array[i];
				memcpy(start++, src.code, sizeof(char[8]));
				latest[i] = i;
				dst = src;
			}
			num_codes = num_rows;
    	}
    	else
    	{
    		for(size_t i = 0; i < count; i++)
    		{	
    			auto& src = array[i];
    			int target = find_row(src.code);
    			auto& old = book[latest[target]];
    			if(target != -1 &&( src.time > old.time || src.volume > old.volume))
    			{
    				if(src.time > old.time) std::cout<<old.time<<" "<<src.time<<std::endl;
    				if(src.volume > old.volume) std::cout<<old.volume<<" "<<src.volume<<std::endl<<std::endl;
    				/// if(src.volume > old.volume) py::print("volume");
    				auto& dst = book[num_rows];
    				dst = src;
    				latest[target] = num_rows;
    				num_rows++;
    			}
    		}
    	}
    }
};
