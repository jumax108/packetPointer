#pragma once

//#define OBJECT_FREE_LIST_TLS_DEBUG

namespace objectFreeListTLS{
	// T type data �ּҿ� �� ���� ���ϸ� node �ּҰ� �˴ϴ�.
	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		constexpr __int64 DATA_TO_NODE_PTR = -8;
	#else
		constexpr __int64 DATA_TO_NODE_PTR = 0;
	#endif

	// �ϳ��� ûũ���� ����� T type ����� �� �Դϴ�.
	constexpr int CHUNK_SIZE = 1000;

	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)
		constexpr unsigned __int64 UNDERFLOW_CHECK_VALUE = 0xAABBCCDDDDCCBBAA;
		constexpr unsigned __int64 OVERFLOW_CHECK_VALUE  = 0xFFEEDDCCCCDDEEFF;
	#endif

	#if defined(OBJECT_FREE_LIST_TLS_DEBUG)

		// alloc object�� �����ϴ� �ִ� ������ ��
		constexpr int MAX_THREAD_NUM = 500;

	#endif

};