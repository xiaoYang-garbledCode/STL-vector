#include<iostream>
template<typename T>
class vector_simple
{
public:
	vector_simple(int size = 10) // ���캯��
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

	vector_simple(const vector_simple<T>& vec) //��������   Ҫ����ǳ��������
	{
		// ָ������õ��Ľ����ָ��֮���Ԫ�ظ���
		int size = vec.end_ - vec.first_; // �ռ��С
		first_ = new T[size];

		int len = vec.last_ - vec.first_; // ��Ч���ݳ���
		
		//���ݿ���
		for (int i = 0; i < len; ++i)
		{
			first_[i] = vec.first_[i];
		}
		last_ = first_ + len;
		end_ = first_ + size;
	}

	vector_simple<T>& operator=(const vector_simple<T>& vec)
	{
		// ��ֹ�Ը�ֵ
		if (this == &vec)
			return *this;

		// ɾ����ǰ������
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

	void push_back(const T& val) // ������β�����Ԫ��
	{
		if (full()) // ������Ҫ����
		{
			expand();
		}
		*last_++ = val;
	}

	void pop_back() // ������ĩβɾȥԪ��
	{
		if (empty())
		{
			return;
		}
		--last_;
	}

	T back() const // ��������ĩβԪ��
	{
		return *(last_ - 1);
	}

private:
	void expand() // �����Ķ�������
	{
		int size = end_ - first_;
		T* temp = new T[size * 2];
		// ���ݿ���
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
	//�����ڴ濪��
	T* allocate(int size)
	{
		// malloc���ֽڿ����ڴ�
		return (T*)malloc(sizeof(T) * size);
	}
	// �����ڴ��ͷ�
	void deallocate(void *p) // �����ڴ��ͷ�
	{
		free(p);
	}
	//�ڿ��ٺõ��ڴ��Ͻ��ж���Ĺ���
	void construct(T* p, const T& val)
	{
		new (p)T(val); // ��λnew
	}
	//������������
	void destroy(T* p)
	{
		p->~T(); 
	}
};

template <typename T, typename Alloc = Allocator<T>> //�û�����ָ���Ϳ�����Ĭ�ϵ�
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
		// ����������Ч��Ԫ�أ�Ȼ���ͷ�first_ָ��ָ����ڴ�
		for (T* p = first_; p != last_; p++)
		{
		
			allocator_.destroy(p);
		}
		// �ͷ��ڴ�
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
		// �ڴ���ÿ��λ�ö�Ҫ�����
		allocator_.allocate(size);
		// ������ЧԪ��
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
		//��ֹ�Կ���
		if (this == &vec)
		{
			return *this;
		}

		// ������ЧԪ��
		for (T* p = first_; p != last_; p++)
		{
			allocator_.destroy(p);
		}
		allocator_.deallocate(first_); // �ͷ�ԭ������ռ�õ��ڴ�

		//����������
		int len = vec.last_ - vec.first_; //��Ч����
		int size = vec.end_ - vec.first_; 
		// �����뿽������ͬ����С���ڴ�
		first_ = allocator_.allocate(size);
		for (int i = 0; i < len; i++)
		{
			// ����������Ч����
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
			expand(); //����
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
		last_--; // �ȼ�1
		allocator_.destroy(last_); //��������
	}

	// ������һ��ʵ�ֳ�������Ƕ������
	class iterator
	{
	public:
		friend class vector<T, Alloc>;
		iterator(vector<T, Alloc>* pVec= nullptr, T* ptr = nullptr)
			:pVec_(pVec), ptr_(ptr) 
		{
			// ͷ�巨
			iterator_base* itb = new iterator_base(this, pVec_->head_.next_);
			pVec_->head_.next_ = itb;
		}
		bool operator!=(const iterator& it)const
		{
			// ������������Ч�� ��ͬ�����ĵ��������ܽ��бȽ�
			if (pVec_ == nullptr || pVec_ != it.pVec_)
			{
				throw "iterator incompatable!";
			}
			return it.ptr_ != ptr_;
		}
		void operator++()
		{
			if (pVec_ == nullptr) // ������������Ч��
			{
				throw "iterator invalid!";
			}
			ptr_++;
		}
		T& operator*() const
		{
			if (pVec_ == nullptr) // ������������Ч��
			{
				throw "iterator invalid!";
			}
			return *ptr_;
		}
		// ��һ��const ��ʾ����ֵ��һ���������ã�����ͨ�����������޸���ָ���Ԫ��ֵ��
		// �ڶ�����ʾ�ǳ����������ں������ڲ����޸Ķ���ĳ�Ա����
		//const T& operator*() const
		//{
		//	if (pVec_ == nullptr) // ������������Ч��
		//	{
		//		throw "iterator invalid!";
		//	}
		//	return *ptr_;
		//}
	private:
		T* ptr_;
		// ��ǰ������ָ������ĸ���������
		vector<T, Alloc>* pVec_;
	};

	//��Ҫ�������ṩbegin��end����
	iterator begin() { return iterator(this, first_); } // this ��ָ��ǰ������ָ��
	iterator end() { return iterator(this, end_); }

	//��������ʧЧ
	void verify(T* first, T* last)
	{
		iterator_base* pre = &this->head_;
		iterator_base* it = pre->next_;
		while (it)
		{
			if (it->cur_->ptr_ > first && it->cur_->ptr_ < last)
			{
				// �õ�������ʧЧ�ķ�Χ��,ɾ��it��㣬����preָ��it����next
				pre->next_ = it->next_;
				it->cur_->ptr_ = nullptr;
				delete it;
				it = pre->next_; // while��it�жϣ���Ӧ����itȥ����
			}
			else
			{
				//���ڷ�Χ�ڣ�����������һ�����
				pre = it;
				it = it->next_;
			}
		}
	}

	// �Զ�������insert����ʵ��
	iterator insert(iterator it, const T& val)
	{
		/*
		* 1.����������
		* 2.������it.ptr_ָ��ĺϷ���
		*/
		verify(it.ptr_ - 1, last_); //��������Χ�ڵĵ�����������ʧЧ
		T* p = last_; //���һ��Ԫ�صĺ��λ��
		while (p > it.ptr_) //��it.ptr(ָ��ǰ����λ��) ��last_-1��Ԫ��ȫ��������(�漰����������)
		{
			allocator_.construct(p, *(p - 1)); // ��ÿ��Ԫ�صĺ���λ��(p)�����Լ�(����)
			allocator_.destroy(p - 1);//�ڵ�ǰλ��(p-1)�����Լ�
			p--; //�Ӻ���ǰ��������
		}
		allocator_.construct(p, val); // �ڵ�ǰλ�ù���Ҫ����Ķ���
		last_++;
		return iterator(this, p); // ����pλ�����ɵ��µĵ�����
	}

	// �Զ�������erase����ʵ��
	iterator erase(iterator it)
	{
		//��������Χ�ĵ������������Ƕ�ʧЧ�� -1��Ϊ���õ�ǰ��������������ҲʧЧ��
		verify(it.ptr_ - 1, last_); 
		T* p = it.ptr_; // ��ǰ�ĺ���һ��λ�� 
		while (p < last_-1)
		{
			//����ǰλ��֮��Ķ���ȫ��ǰ��һ��(���� ����)
			allocator_.destroy(p);
			allocator_.construct(p, *(p + 1));
			p++;
		}
		allocator_.destroy(p); //�������һ������whileѭ�������һ������û����
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
		// �Ѷ��󿽱������ڴ�
		int size = end_ - first_;
		T* temp = allocator_.allocate(size * 2);
		// ����֮ǰ����Ч���󣬲��ͷ��ڴ�
		for (int i=0; i<size; i++)
		{
			// ���µ�λ�ù�����Ч����
			allocator_.construct(temp + i, *(first_ + i));
		}
		for (T* p = first_; p != end_; p++)
		{
			//����֮ǰ����Ч���󣬲��ͷ��ڴ�
			allocator_.destroy(p);
		}
		first_ = temp;
		last_ = first_ + size;
		end_ = first_ + 2 * size;
	}
private:
	// Ϊ�������������ʧЧ����
	struct iterator_base
	{
		iterator_base(iterator * cur =nullptr, iterator_base * next=nullptr)
			:cur_(cur), next_(next){}
		iterator_base* next_;
		iterator* cur_; // ָ��ĳ����������ָ��
	};

	iterator_base head_; //ͷ��㣬�γ�������¼�û��������ֻ�ȡ�����ĸ�Ԫ�صĵ�������

	T* first_;
	T* last_;
	T* end_;
	Alloc allocator_; // ���������ռ�����������
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


//����������
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
	//// ָ������õ��Ľ����ָ��֮���Ԫ�ظ��� 4��
	//int elementsBetween = ptr2 - ptr1;
	//std::cout << "Ԫ��֮��ĸ���: " << elementsBetween<< std::endl;
	//return 0;
