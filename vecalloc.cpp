#include<iostream>
template<typename T>
class vector_simple
{
public:
	vector_simple(int size = 10) // 构造函数
	{
		first_ = new T[size];
		last_ = first_;
		end_ = first_ + size;
	}
	~vector_simple()
	{
		delete[] first_;
		first_ = last_ = end_ = nullptr;
	}

	vector_simple(const vector_simple<T>& vec) //拷贝构造   要拷贝浅拷贝问题
	{
		// 指针相减得到的结果是指针之间的元素个数
		int size = vec.end_ - vec.first_; // 空间大小
		first_ = new T[size];

		int len = vec.last_ - vec.first_; // 有效数据长度
		
		//数据拷贝
		for (int i = 0; i < len; ++i)
		{
			first_[i] = vec.first_[i];
		}
		last_ = first_ + len;
		end_ = first_ + size;
	}

	vector_simple<T>& operator=(const vector_simple<T>& vec)
	{
		// 防止自赋值
		if (this == &vec)
			return *this;

		// 删除当前的数组
		delete[] first_;

		int size = vec.end_ - vec.first_;
		int len = vec.last_ - vec.first_;
		for (int i = 0; i < len; ++i)
		{
			first_[i] = vec.first_[i];
		}
		last_ = first_ + len;
		end_ = first_ + size;
		return *this;
	}

	bool full() const {return last_ == end_;}

	bool empty() const{return last_ == first_;}

	int size() const { return last_ - first_; }

	void push_back(const T& val) // 向容器尾部添加元素
	{
		if (full()) // 满了需要扩容
		{
			expand();
		}
		*last_++ = val;
	}

	void pop_back() // 从容器末尾删去元素
	{
		if (empty())
		{
			return;
		}
		--last_;
	}

	T back() const // 返回容器末尾元素
	{
		return *(last_ - 1);
	}

private:
	void expand() // 容器的二倍扩容
	{
		int size = end_ - first_;
		T* temp = new T[size * 2];
		// 数据拷贝
		for (int i = 0; i < size; i++)
		{
			temp[i] = first_[i];
		}
		delete[] first_;
		first_ = temp;
		last_ = first_ + size;
		end_ = first_ + 2 * size;
	}

private:
	T* first_;
	T* last_;
	T* end_;
};

class Test
{
public:
	Test() { std::cout << "Test()" << std::endl; }
	~Test() { std::cout << "~Test()" << std::endl; }
};

template<typename T>
struct Allocator
{
	//负责内存开辟
	T* allocate(int size)
	{
		// malloc按字节开辟内存
		return (T*)malloc(sizeof(T) * size);
	}
	// 负责内存释放
	void deallocate(void *p) // 负责内存释放
	{
		free(p);
	}
	//在开辟好的内存上进行对象的构造
	void construct(T* p, const T& val)
	{
		new (p)T(val); // 定位new
	}
	//负责对象的析构
	void destroy(T* p)
	{
		p->~T(); 
	}
};

template <typename T, typename Alloc = Allocator<T>> //用户不用指定就可以用默认的
class vector
{
public:
	vector(int size = 10)
	{
		first_ = allocator_.allocate(size);
		last_ = first_;
		end_ = first_ + size;
	}

	~vector()
	{
		// 析构容器有效的元素，然后释放first_指针指向的内存
		for (T* p = first_; p != last_; p++)
		{
		
			allocator_.destroy(p);
		}
		// 释放内存
		allocator_.deallocate(first_);
		first_ = last_ = end_;
	}

	T& operator[](int index)
	{
		if (index < 0 || index >= size())
		{
			throw "index outofrange!";
		}
		return first_[index];
	}

	vector(const vector<T>& vec)
	{
		int len = vec.last_ - vec.first_;
		int size = vec.end_ - vec.first_;
		// 内存是每个位置都要分配的
		allocator_.allocate(size);
		// 拷贝有效元素
		for (int i = 0; i < len; i++)
		{
			//first_[i] = vec.first_[i];
			allocator_.construct(first_ + i, vec.first_[i]);
		}
		last_ = first_ + len;
		end_ = first_ + size;
	}

	vector<T>& operator=(const vector<T>& vec)
	{
		//防止自拷贝
		if (this == &vec)
		{
			return *this;
		}

		// 析构有效元素
		for (T* p = first_; p != last_; p++)
		{
			allocator_.destroy(p);
		}
		allocator_.deallocate(first_); // 释放原先容器占用的内存

		//拷贝的容器
		int len = vec.last_ - vec.first_; //有效对象
		int size = vec.end_ - vec.first_; 
		// 开辟与拷贝容器同样大小的内存
		first_ = allocator_.allocate(size);
		for (int i = 0; i < len; i++)
		{
			// 拷贝构造有效对象
			allocator_.construct(first_ + i, vec.first_[i]);
		}
		last_ = first_ + len;
		end_ = first_ + size;
		return *this;
	}

	void push_back(T val)
	{
		if (full())
		{
			expand(); //扩容
		}
		allocator_.construct(last_, val);
		last_++;
	}

	void pop_back()
	{
		if (empty())
		{
			return;
		}
		last_--; // 先减1
		allocator_.destroy(last_); //对象析构
	}

	// 迭代器一般实现成容器的嵌套类型
	class iterator
	{
	public:
		friend class vector<T, Alloc>;
		iterator(vector<T, Alloc>* pVec= nullptr, T* ptr = nullptr)
			:pVec_(pVec), ptr_(ptr) 
		{
			// 头插法
			iterator_base* itb = new iterator_base(this, pVec_->head_.next_);
			pVec_->head_.next_ = itb;
		}
		bool operator!=(const iterator& it)const
		{
			// 检查迭代器的有效性 不同容器的迭代器不能进行比较
			if (pVec_ == nullptr || pVec_ != it.pVec_)
			{
				throw "iterator incompatable!";
			}
			return it.ptr_ != ptr_;
		}
		void operator++()
		{
			if (pVec_ == nullptr) // 检查迭代器的有效性
			{
				throw "iterator invalid!";
			}
			ptr_++;
		}
		T& operator*() const
		{
			if (pVec_ == nullptr) // 检查迭代器的有效性
			{
				throw "iterator invalid!";
			}
			return *ptr_;
		}
		// 第一个const 表示返回值是一个常量引用，不能通过该引用来修改它指向的元素值。
		// 第二个表示是常量函数，在函数体内不许修改对象的成员变量
		//const T& operator*() const
		//{
		//	if (pVec_ == nullptr) // 检查迭代器的有效性
		//	{
		//		throw "iterator invalid!";
		//	}
		//	return *ptr_;
		//}
	private:
		T* ptr_;
		// 当前迭代器指向的是哪个容器对象
		vector<T, Alloc>* pVec_;
	};

	//需要给容器提供begin和end方法
	iterator begin() { return iterator(this, first_); } // this 是指向当前容器的指针
	iterator end() { return iterator(this, end_); }

	//检查迭代器失效
	void verify(T* first, T* last)
	{
		iterator_base* pre = &this->head_;
		iterator_base* it = pre->next_;
		while (it)
		{
			if (it->cur_->ptr_ > first && it->cur_->ptr_ < last)
			{
				// 该迭代器在失效的范围内,删除it结点，并让pre指向it结点的next
				pre->next_ = it->next_;
				it->cur_->ptr_ = nullptr;
				delete it;
				it = pre->next_; // while用it判断，就应该用it去遍历
			}
			else
			{
				//不在范围内，继续搜索下一个结点
				pre = it;
				it = it->next_;
			}
		}
	}

	// 自定义容器insert方法实现
	iterator insert(iterator it, const T& val)
	{
		/*
		* 1.不考虑扩容
		* 2.不考虑it.ptr_指针的合法性
		*/
		verify(it.ptr_ - 1, last_); //检查这个范围内的迭代器，让其失效
		T* p = last_; //最后一个元素的后继位置
		while (p > it.ptr_) //将it.ptr(指向当前插入位置) 到last_-1的元素全部往后移(涉及构造与析构)
		{
			allocator_.construct(p, *(p - 1)); // 在每个元素的后面位置(p)构造自己(后移)
			allocator_.destroy(p - 1);//在当前位置(p-1)析构自己
			p--; //从后往前析构对象
		}
		allocator_.construct(p, val); // 在当前位置构造要插入的对象
		last_++;
		return iterator(this, p); // 返回p位置生成的新的迭代器
	}

	// 自定义容器erase方法实现
	iterator erase(iterator it)
	{
		//检查这个范围的迭代器，让它们都失效。 -1是为了让当前传入的这个迭代器也失效。
		verify(it.ptr_ - 1, last_); 
		T* p = it.ptr_; // 当前的后面一个位置 
		while (p < last_-1)
		{
			//将当前位置之后的对象全部前移一步(析构 构造)
			allocator_.destroy(p);
			allocator_.construct(p, *(p + 1));
			p++;
		}
		allocator_.destroy(p); //析构最后一个对象，while循环里最后一个对象还没析构
		last_--;
		return iterator(this, it.ptr_);
	}


	T back() const { return *(last_ - 1); }

	bool empty() { return last_ == first_; }
	bool full() { return end_ == last_; }
	int size() { return last_ - first_; }

private:
	void expand()
	{
		// 把对象拷贝到新内存
		int size = end_ - first_;
		T* temp = allocator_.allocate(size * 2);
		// 析构之前的有效对象，并释放内存
		for (int i=0; i<size; i++)
		{
			// 在新的位置构造有效对象
			allocator_.construct(temp + i, *(first_ + i));
		}
		for (T* p = first_; p != end_; p++)
		{
			//析构之前的有效对象，并释放内存
			allocator_.destroy(p);
		}
		first_ = temp;
		last_ = first_ + size;
		end_ = first_ + 2 * size;
	}
private:
	// 为解决容器迭代器失效引入
	struct iterator_base
	{
		iterator_base(iterator * cur =nullptr, iterator_base * next=nullptr)
			:cur_(cur), next_(next){}
		iterator_base* next_;
		iterator* cur_; // 指向某个迭代器的指针
	};

	iterator_base head_; //头结点，形成链表，记录用户从容器种获取的是哪个元素的迭代器。

	T* first_;
	T* last_;
	T* end_;
	Alloc allocator_; // 定义容器空间配置器对象
};
#include<vector>
int main()
{
	vector<int> vec;
	for (int i = 0; i < 20; i++)
	{
		vec.push_back(rand() % 100);
	}
	for (int i = 0; i < 20; i++)
	{
		std::cout << vec[i] << " ";
	}
	vector<int>::iterator it = vec.begin();
	while(it != vec.end())
	{
		if (*it % 2 == 0)
		{
			//it = vec.erase(it);
			it = vec.insert(it, *it + 1);
			++it;
		}
		++it;
		/*else
		{
			++it;
		}*/
	}
	std::cout << std::endl;
	/*for (int val : vec)
	{
		std::cout << val << " ";
	}*/
}


#if 0


//迭代器测试
std::cout << std::endl;
vector<int>::iterator it = vec.begin();
for (; it != vec.end(); ++it)
{
	std::cout << *it << " ";
}

vector<int> vec;
for (int i = 0; i < 20; i++)
{
	vec.push_back(rand() % 100);
}

while (!vec.empty())
{
	std::cout << vec.back() << " ";
	vec.pop_back();
}
#endif


	//char arr[] = { '1', '2', '3', '4', '5' };
	//char* ptr1 = &arr[0];
	//char* ptr2 = &arr[3];
	//// 指针相减得到的结果是指针之间的元素个数 4个
	//int elementsBetween = ptr2 - ptr1;
	//std::cout << "元素之间的个数: " << elementsBetween<< std::endl;
	//return 0;
