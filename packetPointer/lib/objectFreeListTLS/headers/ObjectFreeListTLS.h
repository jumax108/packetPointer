#pragma once

#include <typeinfo>

#include "objectFreeList\headers\objectFreeList.h"

#include "common.h"

#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
	#include "log\headers\log.h"
	#pragma comment(lib, "lib/log/log")
#endif

#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
	#define allocObjectTLS() _allocObject(__FILEW__, __LINE__)
	#define freeObjectTLS(ptr) _freeObject(ptr, __FILEW__, __LINE__)
#else
	#define allocObjectTLS() _allocObject()
	#define freeObjectTLS(ptr) _freeObject(ptr)
#endif

template <typename T>
struct stAllocChunk;

template <typename T>
struct stAllocTlsNode;

template <typename T>
class CObjectFreeListTLS{

public:

	CObjectFreeListTLS(bool runConstructor, bool runDestructor);
	~CObjectFreeListTLS();

	T*	 _allocObject(
		#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
			const wchar_t*, int
		#endif
	);
	void _freeObject (T* object
		#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
			,const wchar_t*, int
		#endif
	);

	unsigned int getCapacity();
	unsigned int getUsedCount();
	
private:
	
	// �� �����忡�� Ȱ���� ûũ�� ����ִ� tls�� index
	unsigned int _allocChunkTlsIdx;

	// ��� �����尡 ���������� �����ϴ� free list �Դϴ�.
	// �̰����� T type�� node�� ū ����� ���ɴϴ�.
	CObjectFreeList<stAllocChunk<T>>* _centerFreeList;
			
	// T type�� ���� ������ ȣ�� ����
	bool _runConstructor;

	// T type�� ���� �Ҹ��� ȣ�� ����
	bool _runDestructor;

	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		// ûũ ����Ʈ �迭�� �ε����� ��� �ִ� tls index
		unsigned int _chunkListTlsIdx;

		struct stSimpleList{
			stAllocChunk<T>* _chunk;
			stSimpleList* _next;
		};

		int _chunkListIndex;
		stSimpleList* _chunkListArr[objectFreeListTLS::MAX_THREAD_NUM];

		HANDLE _heap;

		CLog log;
	#endif
};

template <typename T>
CObjectFreeListTLS<T>::CObjectFreeListTLS(bool runConstructor, bool runDestructor){

	
	///////////////////////////////////////////////////////////////////////
	// �߾� ó���� freeList ����
	_centerFreeList = new CObjectFreeList<stAllocChunk<T>>(runConstructor, runDestructor);
	///////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////
	// tls �Ҵ� ����
	_allocChunkTlsIdx = TlsAlloc();
	if(_allocChunkTlsIdx == TLS_OUT_OF_INDEXES){
		CDump::crash();
	}

	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		_chunkListTlsIdx = TlsAlloc();
		if(_chunkListTlsIdx == TLS_OUT_OF_INDEXES){
			CDump::crash();
		}
	#endif
	///////////////////////////////////////////////////////////////////////

	_runConstructor = runConstructor;
	_runDestructor	= runDestructor;

	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)

		_heap = HeapCreate(0, 0, 0);
	
		_chunkListIndex = 0;

		for(int chunkListCnt = 0; chunkListCnt < objectFreeListTLS::MAX_THREAD_NUM; ++chunkListCnt){
			// ûũ ����Ʈ�� tls���� ȹ���ϰ� �����ϴµ� ��� null�̸� index�� ��� ������ų ���̱� ������ ���� ��带 �־���
			_chunkListArr[chunkListCnt] = (stSimpleList*)HeapAlloc(_heap, HEAP_ZERO_MEMORY, sizeof(stSimpleList));
		}

		log.setDirectory(L"objectFreeListTLS_Log");
		log.setPrintGroup(LOG_GROUP::LOG_DEBUG);

	#endif

}

template <typename T>
CObjectFreeListTLS<T>::~CObjectFreeListTLS(){

	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)

		log(L"memoryLeak.txt", LOG_GROUP::LOG_DEBUG, L"��������������������������������������������������������������������������������������");
		
		wchar_t typeName[50];
		size_t convertSize;
		mbstowcs_s(&convertSize, typeName, typeid(T).name(), 50);
		log(L"memoryLeak.txt", LOG_GROUP::LOG_DEBUG, L"type: %s", typeName);

		for(int listCnt = 0; listCnt < objectFreeListTLS::MAX_THREAD_NUM; ++listCnt){
			stSimpleList* list = _chunkListArr[listCnt];
			while(list->_chunk != nullptr){

				stAllocChunk<T>* chunk = list->_chunk;
				stAllocTlsNode<T>* nodeArr = chunk->_nodes;

				for(int dataCnt = 0; dataCnt < objectFreeListTLS::CHUNK_SIZE; ++dataCnt){

					stAllocTlsNode<T>* node = &nodeArr[dataCnt];

					if(nodeArr[dataCnt]._allocated == true){

						log(L"memoryLeak.txt", LOG_GROUP::LOG_DEBUG, L"alloc file name: %s", node->_allocSourceFileName);
						log(L"memoryLeak.txt", LOG_GROUP::LOG_DEBUG, L"alloc line: %d", node->_allocLine);
						log(L"memoryLeak.txt", LOG_GROUP::LOG_DEBUG, L"data address: 0x%016I64X", &node->_data);
						log(L"memoryLeak.txt", LOG_GROUP::LOG_DEBUG, L"data(%d bytes): ", sizeof(T));

						wchar_t byteLine[25];
						wchar_t* lineWritePoint = byteLine;
						for(int byteCnt = 0; byteCnt < sizeof(T); byteCnt++){

							swprintf_s(lineWritePoint, 4, L"%02X ", *( ((unsigned char*)(&node->_data) + byteCnt)) );
							lineWritePoint += 3;

							if((byteCnt+1) % 8 == 0 || byteCnt+1 == sizeof(T)){
								log(L"memoryLeak.txt", LOG_GROUP::LOG_DEBUG, L"%s", byteLine);
								lineWritePoint = byteLine;
							}
						}

						log(L"memoryLeak.txt", LOG_GROUP::LOG_DEBUG, L"");
					}

				}

				list = list->_next;

			}
		}
		
		log(L"memoryLeak.txt", LOG_GROUP::LOG_DEBUG, L"��������������������������������������������������������������������������������������");

		TlsFree(_chunkListTlsIdx);
		HeapDestroy(_heap);

	#endif

	TlsFree(_allocChunkTlsIdx);
	delete _centerFreeList;
}

template <typename T>
typename T* CObjectFreeListTLS<T>::_allocObject(
	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		const wchar_t* fileName,
		int line
	#endif
){
	
	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		///////////////////////////////////////////////////////////////////////
		// ûũ ����Ʈ ȹ��
		stSimpleList* list = nullptr;
		int chunkListIndex = (int)TlsGetValue(_chunkListTlsIdx);
		if(chunkListIndex == 0){
			int index = InterlockedIncrement((LONG*)&_chunkListIndex);
			TlsSetValue(_chunkListTlsIdx, (void*)index);
			chunkListIndex = index;
		}
		list = _chunkListArr[chunkListIndex];
		///////////////////////////////////////////////////////////////////////
	#endif

	///////////////////////////////////////////////////////////////////////
	// ûũ ȹ��
	stAllocChunk<T>* chunk = (stAllocChunk<T>*)TlsGetValue(_allocChunkTlsIdx);
	if(chunk == nullptr){
		chunk = _centerFreeList->allocObject();
		chunk->init();
		TlsSetValue(_allocChunkTlsIdx, chunk);
		
		#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
			stSimpleList* node = (stSimpleList*)HeapAlloc(_heap, HEAP_ZERO_MEMORY, sizeof(stSimpleList));
			node->_chunk = chunk;
			node->_next = list;
			list = node;
			_chunkListArr[chunkListIndex] = node;
		#endif

	}
	///////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////
	// ûũ���� ��� ȹ�� & �ʱ�ȭ
	stAllocTlsNode<T>* allocNode = chunk->_allocNode;
	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		allocNode->_allocated = true;
		allocNode->_allocSourceFileName = fileName;
		allocNode->_allocLine = line;
	#endif
	///////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////
	// ��忡�� T type ������ ȹ��
	T* allocData = &allocNode->_data;

	if(_runConstructor == true){
	//	new (allocData) T();
	}
	///////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////
	// ���� ��� �ٶ󺸱�
	chunk->_allocNode += 1;
	///////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////
	// ûũ�� ��� ����ߴٸ� ���� �Ҵ�ޱ�
	if(chunk->_allocNode == chunk->_nodeEnd){

		chunk = _centerFreeList->allocObject();
		chunk->init();
		TlsSetValue(_allocChunkTlsIdx, chunk);
		
		#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
			stSimpleList* node = (stSimpleList*)HeapAlloc(_heap, HEAP_ZERO_MEMORY, sizeof(stSimpleList));
			node->_chunk = chunk;
			node->_next = list;
			_chunkListArr[chunkListIndex] = node;
		#endif

	}
	///////////////////////////////////////////////////////////////////////

	return allocData;

}

template <typename T>
void CObjectFreeListTLS<T>::_freeObject(T* object
	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		,const wchar_t* fileName,
		int line
	#endif
){
	
	///////////////////////////////////////////////////////////////////////
	// �Ҵ��ߴ� ��� ȹ��
	stAllocTlsNode<T>* allocatedNode = (stAllocTlsNode<T>*)((unsigned __int64)object + objectFreeListTLS::DATA_TO_NODE_PTR);
	///////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////
	// object ���� ��� ���� üũ
	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		if(allocatedNode->_underflowCheck != objectFreeListTLS::UNDERFLOW_CHECK_VALUE){
			// underflow
			CDump::crash();
		}
		if(allocatedNode->_overflowCheck != objectFreeListTLS::OVERFLOW_CHECK_VALUE){
			// overflow
			CDump::crash();
		}
		if(allocatedNode->_allocated == false){
			// �Ҵ���� ���� ��� ���� �õ�
			CDump::crash();
		}
		allocatedNode->_allocated = false;
	#endif
	///////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////
	// ���� ��û�� �ҽ� ���� & ���� ����
	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		allocatedNode->_freeSourceFileName = fileName;
		allocatedNode->_freeLine = line;
	#endif
	///////////////////////////////////////////////////////////////////////

	
	///////////////////////////////////////////////////////////////////////
	// �Ҹ��� ȣ��
	if(_runDestructor == true){
	//	object->~T();
	}
	///////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////
	// ��忡�� ûũ ȹ��
	stAllocChunk<T>* chunk = allocatedNode->_afflicatedChunk;
	///////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////
	// ûũ�� ��� ��Ұ� ��� �Ϸ�(�Ҵ� �� ��ȯ)�Ǿ��ٸ� ûũ�� ��ȯ
	int leftFreeCnt = InterlockedDecrement((LONG*)&chunk->_leftFreeCnt);
	if(leftFreeCnt == 0){
		chunk->_leftFreeCnt = chunk->_nodeNum;
		chunk->_allocNode = chunk->_nodes;
		_centerFreeList->freeObject(chunk);
	}
	///////////////////////////////////////////////////////////////////////
}

template <typename T>
unsigned int CObjectFreeListTLS<T>::getUsedCount(){
	
	return _centerFreeList->getUsedCount();

}

template <typename T>
unsigned int CObjectFreeListTLS<T>::getCapacity(){
	
	return _centerFreeList->getCapacity();

}

template <typename T>
struct stAllocTlsNode{
	
	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		unsigned __int64 _underflowCheck;
	#endif

	T _data;

	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		unsigned __int64 _overflowCheck;

		const wchar_t* _allocSourceFileName;
		const wchar_t*  _freeSourceFileName;

		int _allocLine;
		int  _freeLine;

		bool _allocated;

	#endif

	// ��尡 �ҼӵǾ��ִ� ûũ
	stAllocChunk<T>* _afflicatedChunk;

	void init(stAllocChunk<T>* afflicatedChunk);
};

template <typename T>
void stAllocTlsNode<T>::init(stAllocChunk<T>* afflicatedChunk){
	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		_underflowCheck = objectFreeListTLS::UNDERFLOW_CHECK_VALUE;
		_overflowCheck  = objectFreeListTLS::OVERFLOW_CHECK_VALUE;

		_allocSourceFileName = nullptr;
		_freeSourceFileName = nullptr;

		_allocLine = 0;
		_freeLine = 0;
	
		_allocated = false;
	#endif

	_afflicatedChunk = afflicatedChunk;
}


template <typename T>
struct stAllocChunk{
		
public:

	int _leftFreeCnt;
	stAllocTlsNode<T>* _allocNode;

	stAllocTlsNode<T> _nodes[objectFreeListTLS::CHUNK_SIZE];	
	stAllocTlsNode<T>* _nodeEnd;

	int _nodeNum;

	void init();

};

template<typename T>
void stAllocChunk<T>::init(){
	
	_nodeNum = objectFreeListTLS::CHUNK_SIZE;

	_allocNode = _nodes;
	_nodeEnd   = _nodes + _nodeNum;

	_leftFreeCnt = _nodeNum;

	for(int nodeCnt = 0; nodeCnt < _nodeNum; ++nodeCnt){
		_nodes[nodeCnt].init(this);
	}
}