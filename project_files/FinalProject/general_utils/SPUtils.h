#ifndef SPUTILS_H_
#define SPUTILS_H_

#include "../SPLogger.h"

/*
 * This file is used to for general utilities, to wrap common actions such as memory
 * allocation and cleaning, checking actions and logging errors
 */


#define ERROR_ALLOCATING_MEMORY 			"Could not allocate memory"
#define ERROR_INVALID_ARGUMENT 				"Error Invalid argument"
#define WARNING_INVALID_ARGUMENT			"Invalid arguments at non critical method"

#define WARNING_FREE_NULL		 			"Trying to free a null pointer"
#define MAX_PATH_LEN                        1025 // 1024 from project specs + 1 for '\0'
#define epsilon 							0.0000000001 //used for double comparison


/* -------------------------- General actions validation  -------------------------------
 * General wrappers for an action that needed to be verified, throw an error (to logger)
 * run some callback and return some value
 *
 * Val = Validate action and log error
 * Rn  = Return NULL
 * Wc  = With callback
 * RCb = Return callback
 * Nc  = Not critical, instead of errors we will print warnings
 *
 * ------------------------------------------------------------------------------------*/

/*
 * The wrapper is used for running 'action' that can fail at our logic,
 * in case the action fails, it reports a warning and run some callback,
 * otherwise it runs a success callback
 *
 * @param action - the action to verify
 * @param warningOnError - the warning message to log when action = false
 * @param onError - the callback code to run in case of error
 * @param onSuccess - the callback code to run in case of success
 */
#define spValWarning(action, warningOnError, onError, onSuccess) do { \
                if(!((action))) { \
					spLoggerSafePrintWarning(warningOnError, __FILE__, __FUNCTION__, __LINE__); \
					onError; \
                } \
                else {\
					onSuccess;\
				}\
        } while (0)

/*
 * The wrapper is used for running 'action' , verifying that its value is true,
 * and if it is not true, pushing error message to log, and returning a fail return value
 *
 * @param action - the action to verify
 * @param errorMessage - the error message to log when action = false
 * @param returnValue - the return value in case of error
 */
#define spVal(action, errorMessage, returnValue) do { \
                if(!((action))) { \
					spLoggerSafePrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return returnValue; \
                } \
        } while (0)

/*
 * The wrapper is used for running 'action' , verifying that its value is true,
 * and if it is not true, pushing warning message to log, and returning a fail return value
 *
 * @param action - the action to verify
 * @param warningMessage - the warning message to log when action = false
 * @param returnValue - the return value in case of error
 */
#define spValNc(action, warningMessage, returnValue) do { \
                if(!((action))) { \
					spLoggerSafePrintWarning(warningMessage, __FILE__, __FUNCTION__, __LINE__); \
					return returnValue; \
                } \
        } while (0)

/*
 * The wrapper is used for running 'action' , verifying that its value is true,
 * and if it is not true, pushing error message to log, and returning NULL
 *
 * @param action - the action to verify
 * @param errorMessage - the error message to log when action = false
 */
#define spValRn(action, errorMessage) do { \
                if(!((action))) { \
					spLoggerSafePrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return NULL; \
                } \
        } while (0)

/*
 * The wrapper is used for running 'action' , verifying that its value is true,
 * and if it is not true, pushing error message to log, running some callback code
 * and returning a fail return value
 *
 * @param action - the action to verify
 * @param errorMessage - the error message to log when action = false
 * @param callBack - a code that is executed if action is false
 * @param returnValue - the return value in case of error
 */
#define spValWc(action, errorMessage, callBack, returnValue) do { \
                if(!((action))) { \
                	callBack; \
					spLoggerSafePrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return returnValue; \
                } \
        } while (0)

/*
 * The wrapper is used for running 'action' , verifying that its value is true,
 * and if it is not true, pushing warning message to log, running some callback code
 * and returning a fail return value
 *
 * @param action - the action to verify
 * @param warningMessage - the warning message to log when action = false
 * @param callBack - a code that is executed if action is false
 * @param returnValue - the return value in case of failure
 */
#define spValWcNc(action, warningMessage, callBack, returnValue) do { \
                if(!((action))) { \
                	callBack; \
					spLoggerSafePrintWarning(warningMessage, __FILE__, __FUNCTION__, __LINE__); \
					return returnValue; \
                } \
        } while (0)

/*
 * The wrapper is used for running 'action' , verifying that its value is true,
 * and if it is not true, pushing error message to log, running some callback code
 * and returning NULL
 *
 * @param action - the action to verify
 * @param errorMessage - the error message to log when action = false
 * @param callBack - a code that is executed if action is false
 */
#define spValWcRn(action, errorMessage, callBack) do { \
                if(!((action))) { \
					callBack; \
					spLoggerSafePrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return NULL; \
                } \
        } while (0)

/*
 * The wrapper is used for running 'action' , verifying that its value is true,
 * and if it is not true, pushing warning message to log, running some callback code
 * and returning NULL
 *
 * @param action - the action to verify
 * @param warningMessage - the warning message to log when action = false
 * @param callBack - a code that is executed if action is false
 */
#define spValWcRnNc(action, warningMessage, callBack) do { \
                if(!((action))) { \
                	callBack; \
					spLoggerSafePrintWarning(warningMessage, __FILE__, __FUNCTION__, __LINE__); \
					return NULL; \
                } \
        } while (0)

/*
 * The wrapper is used for running 'action' , verifying that its value is true,
 * and if it is not true, pushing error message to log, running some callback code
 * and returning its value
 *
 * @param action - the action to verify
 * @param errorMessage - the error message to log when action = false
 * @param callBack - a code that is executed if action is false, and its
 * 						return value is returned.
 */
#define spValRCb(action, errorMessage, callBack) do { \
                if(!((action))) { \
					spLoggerSafePrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return callBack; \
                } \
        } while (0)

/* -------------------------- Arguments validation  ------------------------------------
 * Wrappers for arguments validation, the code is verifying that the arguments
 * validation condition is true,
 * and if it is not true, it pushed arguments error message to log, in addition to
 * extended error that is pushed to the log ,and returns a fail return value
 *
 * Rn  = Return NULL
 * Wc  = With callback
 * Nc  = Not critical, at Nc Wrappers instead of errors, we will print warnings
 *
 * -----------------------------------------------------------------------------------*/

/*
 * The wrapper is used for checking arguments , verifying that the condition is true,
 * and if it is not true, pushing extended error message to log in addition to
 * invalid argument error, and returning a relevant return value
 *
 * @param toVerify - the arguments verification condition
 * @param errorMessage - the error message to log when arguments condition fails
 * @param returnValue - a return value for when the arguments condition fails
 */
#define spVerifyArguments(toVerify, extendedErrorMessage, returnValue) do { \
                if(!((toVerify))) { \
                	spLoggerSafePrintError(ERROR_INVALID_ARGUMENT, __FILE__, __FUNCTION__, __LINE__); \
					spLoggerSafePrintError(extendedErrorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return returnValue; \
                } \
        } while (0)

/*
 * The wrapper is used for checking arguments , verifying that the condition is true,
 * and if it is not true, pushing extended warning message to log in addition to
 * invalid argument warning, and returning a relevant return value
 *
 * @param toVerify - the arguments verification condition
 * @param warningMessage - the warning message to log when arguments condition fails
 * @param returnValue - a return value for when the arguments condition fails
 */
#define spVerifyArgumentsNc(toVerify, warningMessage, returnValue) do { \
                if(!((toVerify))) { \
                	spLoggerSafePrintWarning(WARNING_INVALID_ARGUMENT, __FILE__, __FUNCTION__, __LINE__); \
					spLoggerSafePrintWarning(warningMessage, __FILE__, __FUNCTION__, __LINE__); \
					return returnValue; \
                } \
        } while (0)

/*
 * The wrapper is used for checking arguments , verifying that the condition is true,
 * and if it is not true, pushing to log in invalid argument error,
 * and returning NULL
 * minimal stands for no additional error message
 * @param toVerify - the arguments verification condition
 */
#define spMinimalVerifyArgumentsRn(toVerify) do { \
                if(!((toVerify))) { \
                	spLoggerSafePrintError(ERROR_INVALID_ARGUMENT, __FILE__, __FUNCTION__, __LINE__); \
					return NULL; \
                } \
        } while (0)

/*
 * The wrapper is used for checking arguments , verifying that the condition is true,
 * and if it is not true, pushing to log in invalid argument warning,
 * and returning NULL
 * minimal stands for no additional error message
 *
 * @param toVerify - the arguments verification condition
 */
#define spMinimalVerifyArgumentsRnNc(toVerify) do { \
                if(!((toVerify))) { \
                	spLoggerSafePrintWarning(WARNING_INVALID_ARGUMENT, __FILE__, __FUNCTION__, __LINE__); \
					return NULL; \
                } \
        } while (0)

/*
 * The wrapper is used for checking arguments , verifying that the condition is true,
 * and if it is not true, pushing to log in invalid argument error,
 * and returning a relevant return value
 *minimal stands for no additional error message
 * @param toVerify - the arguments verification condition
 * @param returnValue - a return value for when the arguments condition fails
 */
#define spMinimalVerifyArguments(toVerify, returnValue) do { \
                if(!((toVerify))) { \
                	spLoggerSafePrintError(ERROR_INVALID_ARGUMENT, __FILE__, __FUNCTION__, __LINE__); \
					return returnValue; \
                } \
        } while (0)

/*
 * The wrapper is used for checking arguments , verifying that the condition is true,
 * and if it is not true, pushing extended error message to log in addition to
 * invalid argument error, and returning NULL
 *
 * @param toVerify - the arguments verification condition
 * @param errorMessage - the error message to log when arguments condition fails
 */
#define spVerifyArgumentsRn(toVerify, extendedErrorMessage) do { \
                if(!((toVerify))) { \
                	spLoggerSafePrintError(ERROR_INVALID_ARGUMENT, __FILE__, __FUNCTION__, __LINE__); \
					spLoggerSafePrintError(extendedErrorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return NULL; \
                } \
        } while (0)

/*
 * The wrapper is used for checking arguments , verifying that the condition is true,
 * and if it is not true, pushing extended warning message to log in addition to
 * invalid argument warning, and returning NULL
 *
 * @param toVerify - the arguments verification condition
 * @param warningMessage - the error message to log when arguments condition fails
 */
#define spVerifyArgumentsRnNc(toVerify, warningMessage) do { \
                if(!((toVerify))) { \
                	spLoggerSafePrintWarning(WARNING_INVALID_ARGUMENT, __FILE__, __FUNCTION__, __LINE__); \
					spLoggerSafePrintWarning(warningMessage, __FILE__, __FUNCTION__, __LINE__); \
					return NULL; \
                } \
        } while (0)

/*
 * The wrapper is used for checking arguments , verifying that the condition is true,
 * and if it is not true, pushing extended error message to log in addition to
 * invalid argument error, running some callback code, and returning NULL
 *
 * @param toVerify - the arguments verification condition
 * @param errorMessage - the error message to log when arguments condition fails
 * @param callBack - the callback code to run when arguments condition fails
 */
#define spVerifyArgumentsWcRn(toVerify, extendedErrorMessage, callBack) do { \
                if(!((toVerify))) { \
                	callBack; \
                	spLoggerSafePrintError(ERROR_INVALID_ARGUMENT, __FILE__, __FUNCTION__, __LINE__); \
					spLoggerSafePrintError(extendedErrorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return NULL; \
                } \
        } while (0)

/*
 * The wrapper is used for checking arguments , verifying that the condition is true,
 * and if it is not true, pushing warning message to log in addition to
 * invalid argument warning, running some callback code, and returning NULL
 *
 * @param toVerify - the arguments verification condition
 * @param warningMessage - the warning message to log when arguments condition fails
 * @param callBack - the callback code to run when arguments condition fails
 */
#define spVerifyArgumentsWcRnNc(toVerify, warningMessage, callBack) do { \
                if(!((toVerify))) { \
                	callBack; \
                	spLoggerSafePrintWarning(WARNING_INVALID_ARGUMENT, __FILE__, __FUNCTION__, __LINE__); \
					spLoggerSafePrintWarning(warningMessage, __FILE__, __FUNCTION__, __LINE__); \
					return NULL; \
                } \
        } while (0)

/* -------------------------- Memory allocations  -------------------------------------
 * Wrappers for an memory allocation methods that throw an error (to logger) in case of
 * memory failure, run some callback and return some value
 *
 * Er =  Extended error message
 * Wr  = With return value
 * Wc  = With callback
 * RCb = Return callback
 *
 * -----------------------------------------------------------------------------------*/

/*
 * The wrapper is used for allocating memory, it reports memory allocation failure
 * to the logger, and return NULL in case of failure.
 *
 * @param pointer - a pointer for the allocated memory
 * @param type - the type of the memory allocation
 * @param countOfItems - the count of items to allocate
 */
#define spCalloc(pointer, type, countOfItems)  do { \
				pointer = (type*)calloc(countOfItems, sizeof(type)); \
                if(!(pointer)) { \
					spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					return NULL;\
                } \
        } while (0)


/*
 * The wrapper is used for allocating memory, it reports memory allocation failure
 * to the logger , and return some value in case of failure.
 *
 * @param pointer - a pointer for the allocated memory
 * @param type - the type of the memory allocation
 * @param countOfItems - the count of items to allocate
 * @param returnValue - the value that is returned in case of memory failure
 */
#define spCallocWr(pointer, type, countOfItems, returnValue)  do { \
				pointer = (type*)calloc(countOfItems, sizeof(type)); \
                if(!(pointer)) { \
					spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					return returnValue;\
                } \
        } while (0)

/*
 * The wrapper is used for allocating memory, it reports memory allocation failure
 * to the logger, run's some failure callback code and return NULL in case of failure.
 *
 * @param pointer - a pointer for the allocated memory
 * @param type - the type of the memory allocation
 * @param countOfItems - the count of items to allocate
 * @param onError - the callback code to run in case the allocation fails
 */
#define spCallocWc(pointer, type, countOfItems, onError)  do { \
				pointer = (type*)calloc(countOfItems, sizeof(type)); \
                if(!(pointer)) { \
                	onError; \
					spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					return NULL;\
                } \
        } while (0)

/*
 * The wrapper is used for allocating memory, it reports memory allocation failure
 * to the logger and extended error, run's some failure callback code and
 * NULL in case of failure.
 *
 * @param pointer - a pointer for the allocated memory
 * @param type - the type of the memory allocation
 * @param countOfItems - the count of items to allocate
 * @param errorMessage - the extended error message that is logged in case of failure
 * @param onError - the callback code to run in case the allocation fails
 */
#define spCallocErWc(pointer, type, countOfItems, errorMessage, onError)  do { \
				pointer = (type*)calloc(countOfItems, sizeof(type)); \
                if(!(pointer)) { \
                	onError; \
					spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					spLoggerSafePrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return NULL;\
                } \
        } while (0)

/*
 * The wrapper is used for allocating memory, it reports memory allocation failure
 * to the logger and extended error, run's some failure callback code and
 * returns its value.
 *
 * @param pointer - a pointer for the allocated memory
 * @param type - the type of the memory allocation
 * @param countOfItems - the count of items to allocate
 * @param errorMessage - the extended error message that is logged in case of failure
 * @param onError - the callback code to run (and return) in case the allocation fails
 */
#define spCallocErWcRCb(pointer, type, countOfItems, errorMessage, onError, callBack)  do { \
				pointer = (type*)calloc(countOfItems, sizeof(type)); \
                if(!(pointer)) { \
                	onError; \
					spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					spLoggerSafePrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return callBack;\
                } \
        } while (0)

/*
 * The wrapper is used for allocating memory, it reports memory allocation failure
 * to the logger and extended error,and  return some value in case of failure.
 *
 * @param pointer - a pointer for the allocated memory
 * @param type - the type of the memory allocation
 * @param countOfItems - the count of items to allocate
 * @param errorMessage - the extended error message that is logged in case of failure
 * @param returnValue - the value that is returned in case of memory failure
 */
#define spCallocEr(pointer, type, countOfItems, errorMessage, toReturn)  do { \
				pointer = (type*)calloc(countOfItems, sizeof(type)); \
                if(!(pointer)) { \
					spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					spLoggerSafePrintError(errorMessage, __FILE__, __FUNCTION__, __LINE__); \
					return toReturn;\
                } \
        } while (0)

/*
 * The wrapper is used for re-allocating memory, it reports memory allocation failure
 * to the logger and returns NULL in case of failure.
 *
 * @param pointer - a pointer for the re-allocated memory
 * @param type - the type of the memory re-allocation
 * @param countOfItems - the count of items to re-allocate
 */
#define spRealloc(pointer, type, countOfItems)  do { \
				pointer = ((type)*)realloc((pointer), countOfItems * sizeof(type)); \
                if(!(pointer)) { \
					spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					return NULL;\
                } \
        } while (0)

/*
 * The wrapper is used for re-allocating memory, it reports memory allocation failure
 * to the logger, runs failure callback code and returns NULL in case of failure.
 *
 * @param pointer - a pointer for the re-allocated memory
 * @param type - the type of the memory re-allocation
 * @param countOfItems - the count of items to re-allocate
 * @onError - the callback code to run in case of failure
 */
#define spReallocWc(pointer, type, countOfItems, onError)  do { \
				pointer = (type*)realloc((pointer), countOfItems * sizeof(type)); \
                if(!(pointer)) { \
                	onError; \
					spLoggerSafePrintError(ERROR_ALLOCATING_MEMORY, __FILE__, __FUNCTION__, __LINE__); \
					return NULL;\
                } \
        } while (0)

/* -------------------------- Memory deallocations  -------------------------------------
 * Wrappers for memory cleanup.
 *
 * Safe  = report free(NULL) as warning
 * ------------------------------------------------------------------------------------*/

/*
 * The wrapper is used for freeing a memory by its pointer and setting the pointer to NULL
 * if its not already NULL
 *
 * @param pointer - the pointer for the memory we wish to clean
 */
#define spFree(pointer)  do { \
                if((pointer)) { \
					free(pointer);\
					pointer = NULL;\
                } \
        } while (0)

/*
 * The wrapper is used for freeing a memory by its pointer and setting the pointer to NULL
 * if its not already NULL. the method reports warnings to the logger in case that
 * pointer already NULL.
 *
 * @param pointer - the pointer for the memory we wish to clean
 * @param warningMessage - an extended warning message to add when pointer == NULL
 */
#define spSafeFree(pointer, warningMessage)  do { \
                if((pointer)) { \
					free(pointer);\
					pointer = NULL;\
                } \
                else {\
					spLoggerSafePrintWarning(WARNING_FREE_NULL, __FILE__, __FUNCTION__, __LINE__); \
                	spLoggerSafePrintWarning(warningMessage, __FILE__, __FUNCTION__, __LINE__); \
				}\
        } while (0)

#endif /* SPUTILS_H_ */
