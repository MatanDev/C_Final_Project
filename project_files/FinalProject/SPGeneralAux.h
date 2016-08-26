#ifndef SPGENERALAUX_H_
#define SPGENERALAUX_H_

//TODO - validate and comment

//	'spVal' 	stand for 'verify and log action'
//	'Wc' 		stand for 'with callback'
//	'Rn' 		stands for 'return null'
//	'RCb' 		stand for 'return callback'
#define ERROR_ALLOCATING_MEMORY 								"Could not allocate memory"
#define ERROR_INVALID_ARGUMENT 									"Error Invalid argument"

#define WARNING_FREE_NULL		 								"Trying to free a null pointer"


#define spVal(action, errorMessage, returnValue) do { \
                if(!((action))) { \
					spLoggerPrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return returnValue; \
                } \
        } while (0)

#define spValRn(action, errorMessage) do { \
                if(!((action))) { \
					spLoggerPrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return NULL; \
                } \
        } while (0)

#define spValWc(action, errorMessage, callBack, returnValue) do { \
                if(!((action))) { \
					spLoggerPrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					callBack; \
					return returnValue; \
                } \
        } while (0)

#define spValWcRn(action, errorMessage, callBack) do { \
                if(!((action))) { \
					spLoggerPrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					callBack; \
					return NULL; \
                } \
        } while (0)

#define spValRCb(action, errorMessage, callBack, returnValue) do { \
                if(!((action))) { \
					spLoggerPrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return callBack; \
                } \
        } while (0)

#define spCalloc(pointer, type, countOfItems)  do { \
				pointer = (type*)calloc(countOfItems, sizeof(type)); \
                if(!(pointer)) { \
					spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					return NULL;\
                } \
        } while (0)

#define spCallocWc(pointer, type, countOfItems, onError)  do { \
				pointer = (type*)calloc(countOfItems, sizeof(type)); \
                if(!(pointer)) { \
					spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					onError; \
					return NULL;\
                } \
        } while (0)

#define spRealloc(pointer, type, countOfItems)  do { \
				pointer = (type*)realloc((pointer), countOfItems * sizeof(type)); \
                if(!(pointer)) { \
					spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					return NULL;\
                } \
        } while (0)

#define spReallocWc(pointer, type, countOfItems, onError)  do { \
				pointer = (type*)realloc((pointer), countOfItems * sizeof(type)); \
                if(!(pointer)) { \
					spLoggerPrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					onError; \
					return NULL;\
                } \
        } while (0)

#define spFree(pointer)  do { \
                if(!(pointer)) { \
					free(pointer);\
                } \
        } while (0)

#define spSafeFree(pointer, warningMessage)  do { \
                if(!(pointer)) { \
					free(pointer);\
                } \
                else {\
					spLoggerPrintWarning(WARNING_FREE_NULL, __FILE__, __FUNCTION__, __LINE__); \
                	spLoggerPrintWarning(warningMessage, __FILE__, __FUNCTION__, __LINE__); \
				}\
        } while (0)

#define spFreeByPointer(pointerToPointer)  do { \
                if(!(*pointerToPointer)) { \
					free(*pointerToPointer);\
					*pointerToPointer = NULL;\
                } \
        } while (0)

#define spSafeFreeByPointer(pointerToPointer, warningMessage)  do { \
               if(!(*pointerToPointer)) { \
					free(*pointerToPointer);\
					*pointerToPointer = NULL;\
                } \
                else {\
					spLoggerPrintWarning(WARNING_FREE_NULL, __FILE__, __FUNCTION__, __LINE__); \
                	spLoggerPrintWarning(warningMessage, __FILE__, __FUNCTION__, __LINE__); \
				}\
        } while (0)



#endif /* SPGENERALAUX_H_ */