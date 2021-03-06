CC = gcc
CPP = g++
#put your object files here
OBJS = main.o SPImageProc.o SPPoint.o SPListElement.o SPList.o SPBPriorityQueue.o SPKDArray.o SPKDTreeNode.o SPKDTreeNodeKNN.o \
SPConfig.o SPLogger.o SPImagesParser.o SPMainAux.o SPImageQuery.o SPImageData.o
#The executabel filename
EXEC = SPCBIR
PRIORITY_QUEUE_DIR = ./data_structures/bpqueue_ds
KD_DS_DIR = ./data_structures/kd_ds
IMAGE_PARSING_DIR = ./image_parsing
MAIN_AND_UI_DIR = ./main_and_ui
GENERAL_UTILS_DIR = ./general_utils
INCLUDEPATH=/usr/local/lib/opencv-3.1.0/include/
LIBPATH=/usr/local/lib/opencv-3.1.0/lib/
LIBS=-lopencv_xfeatures2d -lopencv_features2d \
-lopencv_highgui -lopencv_imgcodecs -lopencv_imgproc -lopencv_core


CPP_COMP_FLAG = -std=c++11 -Wall -Wextra \
-Werror -pedantic-errors -DNDEBUG

C_COMP_FLAG = -std=c99 -Wall -Wextra \
-Werror -pedantic-errors -DNDEBUG

$(EXEC): $(OBJS)
	$(CPP) $(OBJS) -L$(LIBPATH) $(LIBS) -o $@
main.o: main.cpp SPConfig.h SPLogger.h SPImageProc.h $(PRIORITY_QUEUE_DIR)/SPBPriorityQueue.h $(IMAGE_PARSING_DIR)/SPImagesParser.h \
			SPPoint.h $(KD_DS_DIR)/SPKDTreeNode.h $(MAIN_AND_UI_DIR)/SPImageQuery.h $(MAIN_AND_UI_DIR)/SPMainAux.h
	$(CPP) $(CPP_COMP_FLAG) -I$(INCLUDEPATH) -c $*.cpp
SPImageProc.o: SPImageProc.cpp SPImageProc.h SPConfig.h SPPoint.h SPLogger.h
	$(CPP) $(CPP_COMP_FLAG) -I$(INCLUDEPATH) -c $*.cpp
#a rule for building a simple c souorce file
#use gcc -MM SPPoint.c to see the dependencies

#------------------------------------------------priority queue--------------------------------------------------------------------------------

SPListElement.o: $(PRIORITY_QUEUE_DIR)/SPListElement.c $(PRIORITY_QUEUE_DIR)/SPListElement.h $(GENERAL_UTILS_DIR)/SPUtils.h
	$(CC) $(C_COMP_FLAG) -c $(PRIORITY_QUEUE_DIR)/$*.c
	
SPList.o: $(PRIORITY_QUEUE_DIR)/SPList.c $(PRIORITY_QUEUE_DIR)/SPList.h $(PRIORITY_QUEUE_DIR)/SPListElement.h $(GENERAL_UTILS_DIR)/SPUtils.h
	$(CC) $(C_COMP_FLAG) -c $(PRIORITY_QUEUE_DIR)/$*.c
		
SPBPriorityQueue.o: $(PRIORITY_QUEUE_DIR)/SPBPriorityQueue.c $(PRIORITY_QUEUE_DIR)/SPBPriorityQueue.h $(PRIORITY_QUEUE_DIR)/SPList.h \
							$(PRIORITY_QUEUE_DIR)/SPListElement.h $(GENERAL_UTILS_DIR)/SPUtils.h
	$(CC) $(C_COMP_FLAG) -c $(PRIORITY_QUEUE_DIR)/$*.c

#--------------------------------------------kd data structure--------------------------------------------------------------------------------

SPKDArray.o: $(KD_DS_DIR)/SPKDArray.c $(KD_DS_DIR)/SPKDArray.h SPLogger.h SPPoint.h $(GENERAL_UTILS_DIR)/SPUtils.h
	$(CC) $(C_COMP_FLAG) -c $(KD_DS_DIR)/$*.c

SPKDTreeNode.o: $(KD_DS_DIR)/SPKDTreeNode.c $(KD_DS_DIR)/SPKDTreeNode.h SPLogger.h $(KD_DS_DIR)/SPKDArray.h SPConfig.h $(GENERAL_UTILS_DIR)/SPUtils.h
	$(CC) $(C_COMP_FLAG) -c $(KD_DS_DIR)/$*.c
	
SPKDTreeNodeKNN.o: $(KD_DS_DIR)/SPKDTreeNodeKNN.c $(KD_DS_DIR)/SPKDTreeNodeKNN.h SPLogger.h $(KD_DS_DIR)/SPKDTreeNode.h \
								$(PRIORITY_QUEUE_DIR)/SPBPriorityQueue.h $(GENERAL_UTILS_DIR)/SPUtils.h
	$(CC) $(C_COMP_FLAG) -c $(KD_DS_DIR)/$*.c
	
SPPoint.o: SPPoint.c SPPoint.h  SPLogger.h $(GENERAL_UTILS_DIR)/SPUtils.h
	$(CC) $(C_COMP_FLAG) -c $*.c

#----------------------------------------------config and logger------------------------------------------------------------------------------

SPConfig.o: SPConfig.c SPConfig.h  SPLogger.h $(GENERAL_UTILS_DIR)/SPUtils.h
	$(CC) $(C_COMP_FLAG) -c $*.c
	
SPLogger.o: SPLogger.c SPLogger.h $(GENERAL_UTILS_DIR)/SPUtils.h
	$(CC) $(C_COMP_FLAG) -c $*.c
	
#-------------------------------------------------------------image parser----------------------------------------------------------------------------------

SPImageData.o: $(IMAGE_PARSING_DIR)/SPImageData.c $(IMAGE_PARSING_DIR)/SPImageData.h  SPPoint.h SPConfig.h $(GENERAL_UTILS_DIR)/SPUtils.h SPLogger.h
	$(CC) $(C_COMP_FLAG) -c $(IMAGE_PARSING_DIR)/$*.c


SPImagesParser.o: $(IMAGE_PARSING_DIR)/SPImagesParser.c $(IMAGE_PARSING_DIR)/SPImagesParser.h  SPPoint.h SPConfig.h $(GENERAL_UTILS_DIR)/SPUtils.h $(IMAGE_PARSING_DIR)/SPImageData.h
	$(CC) $(C_COMP_FLAG) -c $(IMAGE_PARSING_DIR)/$*.c

#---------------------------------------------------main aux and image query------------------------------------------------------------------------

SPMainAux.o: $(MAIN_AND_UI_DIR)/SPMainAux.c $(MAIN_AND_UI_DIR)/SPMainAux.h  $(MAIN_AND_UI_DIR)/SPImageQuery.h SPLogger.h \
					$(KD_DS_DIR)/SPKDTreeNode.h $(PRIORITY_QUEUE_DIR)/SPBPriorityQueue.h $(IMAGE_PARSING_DIR)/SPImagesParser.h SPConfig.h \
					$(GENERAL_UTILS_DIR)/SPUtils.h $(IMAGE_PARSING_DIR)/SPImageData.h
	$(CC) $(C_COMP_FLAG) -c $(MAIN_AND_UI_DIR)/$*.c
	
SPImageQuery.o: $(MAIN_AND_UI_DIR)/SPImageQuery.c $(MAIN_AND_UI_DIR)/SPImageQuery.h  SPConfig.h $(IMAGE_PARSING_DIR)/SPImagesParser.h \
						$(PRIORITY_QUEUE_DIR)/SPBPriorityQueue.h $(KD_DS_DIR)/SPKDTreeNode.h $(KD_DS_DIR)/SPKDTreeNodeKNN.h $(GENERAL_UTILS_DIR)/SPUtils.h
	$(CC) $(C_COMP_FLAG) -c $(MAIN_AND_UI_DIR)/$*.c
		

	
clean:
	rm -f $(OBJS) $(EXEC)
