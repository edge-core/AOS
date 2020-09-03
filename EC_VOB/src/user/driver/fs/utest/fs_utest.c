/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define FS_UTEST
#define FS_SAVE_FOLDER  "test/"

/* test target source code */
#include "fs.c"

#if 1 /* workaround for org code */
#ifndef FS_FILENAME_MARK_SIZE
#define FS_FILENAME_MARK_SIZE 0
#endif
#ifndef FS_FILENAME_MARK
#define FS_FILENAME_MARK ""
#endif
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define ASSERT assert

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void FS_GetDeviceFileName_UT(void);
static void FS_UpdateFile_UT(void);
static void FS_SetStartupFilename_UT();

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static void FS_GetDeviceFileName_UT(void)
{
    BOOL_T rc;
    const char* user_filename="abc.def";
    char full_name[FS_MAX_PATHNAME_BUF_SIZE+FS_FILENAME_MARK_SIZE];
    char full_name_verify[FS_MAX_PATHNAME_BUF_SIZE+FS_FILENAME_MARK_SIZE];
    FS_File_Type_T file_type;
    struct
    {
        const char* prefix_path_p;
        BOOL_T with_mark;
    } verify_data[FS_FILE_TYPE_TOTAL] =
    {
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_SUBFILE*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_KERNEL*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_DIAG*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_RUNTIME*/
        FS_SAVE_FOLDER, TRUE,/*FS_FILE_TYPE_SYSLOG*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_CMDLOG*/
        FS_SAVE_FOLDER, TRUE,/*FS_FILE_TYPE_CONFIG*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_POSTLOG*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_PRIVATE*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_CERTIFICATE*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_ARCHIVE*/
        FS_SAVE_FOLDER, TRUE,/*FS_FILE_TYPE_BINARY_CONFIG*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_PUBLIC*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_CPEFIRMWARE*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_CPECONFIG*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_FILEMAPPING*/
        SYS_ADPT_LICENSE_FILE_PATH, FALSE,/*FS_FILE_TYPE_LICENSE*/
        FS_SAVE_FOLDER, FALSE,/*FS_FILE_TYPE_NOS_INSTALLER*/
    };

    for(file_type=0; file_type<FS_FILE_TYPE_TOTAL; file_type++)
    {
        rc=FS_GetDeviceFileName((UI8_T*)user_filename, file_type, full_name);
        if (rc==FALSE)
            ASSERT(0);
        else
        {
            if(verify_data[file_type].with_mark==TRUE)
            {
                snprintf(full_name_verify, sizeof(full_name_verify), "%s%s"FS_FILENAME_MARK,
                    verify_data[file_type].prefix_path_p,user_filename);
                if (strcmp(full_name, full_name_verify)!=0)
                    ASSERT(0);
            }
            else
            {
                snprintf(full_name_verify, sizeof(full_name_verify), "%s%s",
                    verify_data[file_type].prefix_path_p,user_filename);
                if (strcmp(full_name, full_name_verify)!=0)
                    ASSERT(0);
            }
        }
    }

    return;
}

static void FS_UpdateFile_UT(void)
{
    UI32_T rc;
    char user_filename[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    char full_name[FS_MAX_PATHNAME_BUF_SIZE+FS_FILENAME_MARK_SIZE];
    FS_FileHeader_T file_header;
    FILE *fd;
    struct stat     file_info;
    UI32_T type;
    int i;

    struct
    {
        UI32_T type;
        const char* file_name;
        BOOL_T test_startup;
    } test_data[2] =
    {
        FS_FILE_TYPE_CONFIG, "test.cfg", TRUE,
        FS_FILE_TYPE_NOS_INSTALLER, "runtime", TRUE,
    };

    mkdir(FS_SAVE_FOLDER, 0755);

    for (i=0; i<2; i++)
    {
        /*1: update new file*/
        sprintf(user_filename, test_data[i].file_name);
        type = test_data[i].type;

        if (FS_RETURN_OK != FS_UpdateFile(1, (UI8_T *) user_filename, NULL, type, "new", 10, 10, FALSE))
            ASSERT(0);

        strncpy((char*)file_header.file_name,(char*)user_filename,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
        file_header.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';
        if (FALSE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK_ALL))
        {
            ASSERT(0);
        }

        FS_GetDeviceFileName((UI8_T *) user_filename, type, full_name);
        fd = fopen(full_name, "r");
        if (fd == NULL)
            ASSERT(0);
        fclose(fd);

        /*2: update startup file*/
        if (test_data[i].test_startup)
        {
            FS_SetStartupFilename(DUMMY_DRIVE, type, (UI8_T *) user_filename);
            if (FS_RETURN_OK != FS_UpdateFile(1, (UI8_T *) user_filename, NULL, type, "modify", 10, 10, FALSE))
                ASSERT(0);

            strncpy((char*)file_header.file_name,(char*)user_filename,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
            file_header.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';
            if (FALSE == FS_OM_GetFileHeaderByName(&file_header,FS_FILE_TYPE_MASK_ALL))
                ASSERT(0);
            if (file_header.startup == FALSE)
                ASSERT(0);

            FS_GetDeviceFileName((UI8_T *) user_filename, type, full_name);
            if( stat((char*)full_name, &file_info)!=0) 
                ASSERT(0);
            if( !FS_UID_TO_STARTUP(file_info.st_uid))
                ASSERT(0);
        }

        /*clean data*/
        FS_EraseFile(1, (UI8_T *)user_filename, FALSE);
    }

    rmdir(FS_SAVE_FOLDER);
    printf("%s test success\r\n", __FUNCTION__);
    return;
}

static void FS_SetStartupFilename_UT()
{
    UI32_T rc;
    char user_filename1[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    char user_filename2[SYS_ADPT_FILE_SYSTEM_NAME_LEN+1];
    char full_name1[FS_MAX_PATHNAME_BUF_SIZE+FS_FILENAME_MARK_SIZE];
    char full_name2[FS_MAX_PATHNAME_BUF_SIZE+FS_FILENAME_MARK_SIZE];
    FS_FileHeader_T file_header1;
    FS_FileHeader_T file_header2;
    FILE *fd;
    struct stat     file_info;
    UI32_T type;
    int i;

    struct
    {
        UI32_T type;
        const char* file_name1;
        const char* file_name2;
    } test_data[2] =
    {
        FS_FILE_TYPE_CONFIG, "test1.cfg", "test2.cfg",
        FS_FILE_TYPE_NOS_INSTALLER, "runtime1", "runtime2",
    };

    mkdir(FS_SAVE_FOLDER, 0755);

    for (i=0; i<2; i++)
    {
        /*1: update file1 and set as startup*/
        sprintf(user_filename1, test_data[i].file_name1);
        type = test_data[i].type;

        if (FS_RETURN_OK != FS_UpdateFile(1, (UI8_T *) user_filename1, NULL, type, "new", 10, 10, FALSE))
            ASSERT(0);

        FS_SetStartupFilename(DUMMY_DRIVE, type, (UI8_T *) user_filename1);

        /*check OM*/
        strncpy((char*)file_header1.file_name,(char*)user_filename1,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
        file_header1.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';
        if (FALSE == FS_OM_GetFileHeaderByName(&file_header1,FS_FILE_TYPE_MASK_ALL))
            ASSERT(0);
        if (file_header1.startup == FALSE)
            ASSERT(0);

        /*check file UID*/
        FS_GetDeviceFileName((UI8_T *) user_filename1, type, full_name1);
        if( stat((char*)full_name1, &file_info)!=0) 
            ASSERT(0);
        if( !FS_UID_TO_STARTUP(file_info.st_uid))
            ASSERT(0);

        /*2: update file2 and set as startup*/
        sprintf(user_filename2, test_data[i].file_name2);
        type = test_data[i].type;

        if (FS_RETURN_OK != FS_UpdateFile(1, (UI8_T *) user_filename2, NULL, type, "new", 10, 10, FALSE))
            ASSERT(0);

        FS_SetStartupFilename(DUMMY_DRIVE, type, (UI8_T *) user_filename2);

        /*check OM, file2 startup: TRUE, file1 startup: FALSE*/
        strncpy((char*)file_header2.file_name,(char*)user_filename2,SYS_ADPT_FILE_SYSTEM_NAME_LEN);
        file_header2.file_name[SYS_ADPT_FILE_SYSTEM_NAME_LEN]='\0';
        if (FALSE == FS_OM_GetFileHeaderByName(&file_header2,FS_FILE_TYPE_MASK_ALL))
            ASSERT(0);
        if (file_header2.startup == FALSE)
            ASSERT(0);

        if (FALSE == FS_OM_GetFileHeaderByName(&file_header1,FS_FILE_TYPE_MASK_ALL))
            ASSERT(0);
        if (file_header1.startup == TRUE)
            ASSERT(0);


        /*check file UID*/
        FS_GetDeviceFileName((UI8_T *) user_filename2, type, full_name2);
        if( stat((char*)full_name2, &file_info)!=0) 
            ASSERT(0);
        if( !FS_UID_TO_STARTUP(file_info.st_uid))
            ASSERT(0);

        if( stat((char*)full_name1, &file_info)!=0) 
            ASSERT(0);
        if( FS_UID_TO_STARTUP(file_info.st_uid))
            ASSERT(0);

        /*clean data*/
        FS_EraseFile(1, (UI8_T *)user_filename1, FALSE);
        FS_EraseFile(1, (UI8_T *)user_filename2, FALSE);
    }

    rmdir(FS_SAVE_FOLDER);
    printf("%s test success\r\n", __FUNCTION__);
    return;
}

int main(int argc, char* argv[])
{
    FS_GetDeviceFileName_UT();
    FS_UpdateFile_UT();
    FS_SetStartupFilename_UT();
    return 0;
}
