/*
* Copyright 2020, @Ralph0045
* gcc dtree_patcher.c -o dtree_patcher
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GET_OFFSET(dtree_len, x) (x - (uintptr_t) dtree_buf)

void* memstr(const void* mem, size_t size, const char* str) {
    return (void*) memmem(mem, size, str, strlen(str));
}

void* pattern_search(const void* addr, int len, int pattern, int mask, int step) {
    char* caddr = (char*)addr;
    if (len <= 0)
        return NULL;
    if (step < 0) {
        len = -len;
        len &= ~-(step+1);
    } else {
        len &= ~(step-1);
    }
    for (int i = 0; i != len; i += step) {
        uint32_t x = *(uint32_t*)(caddr + i);
        if ((x & mask) == pattern)
            return (void*)(caddr + i);
    }
    return NULL;
}

int no_effaceable_patch (FILE *fp,size_t dtree_len,void* dtree_buf) {
    printf("no_effaceable_storage_patch:\n");
    void* lwvm_loc = memstr(dtree_buf, dtree_len, "use-lwvm");
    if(!lwvm_loc) {
        printf("Failed to find use-lwvm string\n");
        return -1;
    }
    printf("Found use-lwvm string at %p\n",GET_OFFSET(dtree_len, lwvm_loc));
    
    memcpy(lwvm_loc+0x24,lwvm_loc,ftell(fp)+0x4000);

    void* sacm_loc = memstr(dtree_buf, dtree_len, "\x73\x61\x63\x6D\x00\x00");
    if(!sacm_loc) {
        printf("Failed to find sacm string\n");
        return -1;
    }
    void* next_sacm_loc = memstr(sacm_loc+0x12, dtree_len, "sacm");
    if(!next_sacm_loc) {
        printf("Failed to find next_sacm string\n");
        return -1;
    }
    void* AAPL_phandle_loc = memstr(next_sacm_loc, dtree_len, "AAPL,phandle");
    if(!next_sacm_loc) {
        printf("Failed to find next AAPL,phandle string\n");
        return -1;
    }

    printf("Found AAPL,phandle at %p\n",GET_OFFSET(dtree_len, AAPL_phandle_loc));

    /* Newer devicetrees have at least 10 properties */
    for (unsigned i = 0xA; i <= 0x20; i++) {
        void* property = pattern_search(AAPL_phandle_loc, 0x46, 0x000000+i, 0x0000FFFF, 1);
        if(property) {
            printf("Found %u defaults properties at %p\n",i,GET_OFFSET(dtree_len, property));
            *(uint32_t*)property = __builtin_bswap16((i+0x1)*0x100);
            break;
        }
    }
    
    /* Insert no-effaceable-storage string */
    printf("Adding no-effaceable-storage string at %p\n",GET_OFFSET(dtree_len, lwvm_loc));
    strcpy(lwvm_loc,"no-effaceable-storage");
    return 1;
}

int preboot_role_8002_patch (FILE *fp,size_t dtree_len,void* dtree_buf) {
    printf("preboot_role_8002_patch:\n");
    void* fstab_loc = memstr(dtree_buf, dtree_len, "fstab");
    if(!fstab_loc) {
        printf("Failed to find fstab string\n");
        return -1;
    }
    printf("Found fstab string at %p\n",GET_OFFSET(dtree_len, fstab_loc));
    void* system_loc = memstr(fstab_loc, dtree_len, "System");
    if(!system_loc) {
        printf("Failed to find System string\n");
        return -1;
    }
    printf("Found System string at %p\n",GET_OFFSET(dtree_len, system_loc));
    void* preboot_role_str_loc = memstr(system_loc, dtree_len, "vol.fs_role");
    if(!preboot_role_str_loc) {
        printf("Failed to find Preboot role string loc string\n");
        return -1;
    }
    void* preboot_role = memstr(preboot_role_str_loc, dtree_len, "\x10\x00");
    if(!preboot_role) {
        printf("Failed to find Preboot role loc string\n");
        return -1;
    }
    printf("Found Preboot role at %p\n",GET_OFFSET(dtree_len, preboot_role));
    printf("Changing Preboot role to 0x8002\n");
    memcpy(preboot_role,"\x80\x02",0x2);
    return 1;

}

int data_role_0_patch (FILE *fp,size_t dtree_len,void* dtree_buf) {
    printf("data_role_0_patch:\n");
    void* fstab_loc = memstr(dtree_buf, dtree_len, "fstab");
    if(!fstab_loc) {
        printf("Failed to find fstab string\n");
        return -1;
    }
    printf("Found fstab string at %p\n",GET_OFFSET(dtree_len, fstab_loc));
    void* system_loc = memstr(fstab_loc, dtree_len, "System");
    if(!system_loc) {
        printf("Failed to find system string\n");
        return -1;
    }
    printf("Found System string at %p\n",GET_OFFSET(dtree_len, system_loc));
    void* data_role_str_loc = memstr(system_loc, dtree_len, "vol.fs_role");
    if(!data_role_str_loc) {
        printf("Failed to find data role string loc string\n");
        return -1;
    }
    void* data_role = memstr(data_role_str_loc, dtree_len, "\x40\x00");
    if(!data_role) {
        printf("Failed to find data role loc string\n");
        return -1;
    }
    printf("Found data role at %p\n",GET_OFFSET(dtree_len, data_role));
    printf("Changing data role to 0x00\n");
    memcpy(data_role,"\x00",0x1);
    return 1;
    
}

int mount_as_rw (FILE *fp,size_t dtree_len,void* dtree_buf) {
    printf("mount_as_rw :\n");
    void* fstab_loc = memstr(dtree_buf, dtree_len, "fstab");
    if(!fstab_loc) {
        printf("Failed to find fstab string\n");
        return -1;
    }
    printf("Found fstab string at %p\n",GET_OFFSET(dtree_len, fstab_loc));
    void* xart_loc = memstr(fstab_loc, dtree_len, "xART");
    if(!xart_loc) {
        printf("Failed to find xART string\n");
        return -1;
    }
    printf("Found System string at %p\n",GET_OFFSET(dtree_len, xart_loc));
    void* xart_mount_str_loc = memstr(xart_loc, dtree_len, "vol.fs_type");
    if(!xart_mount_str_loc) {
        printf("Failed to find data role string loc string\n");
        return -1;
    }
    void* data_role = memstr(xart_mount_str_loc, dtree_len, "\x72\x6f");
    if(!data_role) {
        printf("Failed to find mount ro\n");
        return -1;
    }
    printf("Found mount ro at %p\n",GET_OFFSET(dtree_len, data_role));
    printf("Changing mount as 0x7277\n");
    memcpy(data_role,"\x72\x77",0x2);
    return 1;
    
}


int main(int argc, char **argv){
    
    FILE* fp = NULL;
    
    if(argc < 4){
        printf("Usage: %s <dtree_in> <dtree_out> <args>\n",argv[0]);
        printf("\t-n\t\tAdd no-effaceable-storage property\n");
        printf("\t-d\t\tChange data volume role to 0x0\n");
        printf("\t-p\t\tChange Preboot volume role to 'D' (0x8002)\n");
        printf("\t-o\t\tChange System mount as a ro to rw \n");
        return 0;
    }
    
    void* dtree_buf;
    size_t dtree_len;
    
    printf("Opening DeviceTree\n");
    
    fp = fopen(argv[1], "rb");
    if(!fp) {
        printf("%s: Error opening %s!\n", __FUNCTION__, argv[1]);
        return -1;
    }
    
    fseek(fp, 0, SEEK_END);
    dtree_len = ftell(fp)+0x4000;
    fseek(fp, 0, SEEK_SET);
    
    dtree_buf = (void*)malloc(dtree_len);
    if(!dtree_buf) {
        printf("%s: Out of memory!\n", __FUNCTION__);
        fclose(fp);
        return -1;
    }
    
    fread(dtree_buf, 1, dtree_len, fp);
    fclose(fp);
    
    if(memstr(dtree_buf, dtree_len, "dtre")) {
        printf("Detected IM4P, you have to unpack it!\n");
        return -1;
    }
    
    for(int i=0;i<argc;i++) {
        if(strcmp(argv[i], "-n") == 0) {
            no_effaceable_patch(fp,dtree_len,dtree_buf);
        }
        if(strcmp(argv[i], "-d") == 0) {
            data_role_0_patch(fp,dtree_len,dtree_buf);
        }
        if(strcmp(argv[i], "-p") == 0) {
            preboot_role_8002_patch(fp,dtree_len,dtree_buf);
        }
        if(strcmp(argv[i], "-o") == 0) {
            mount_as_rw(fp,dtree_len,dtree_buf);
        }
    }
    
    /* Write patched dtree */
    printf("Writing patched devicetree\n");
    
    fp = fopen(argv[2], "wb+");
    if(!fp) {
        printf("%s: Unable to open %s!\n", __FUNCTION__, argv[2]);
        free(dtree_buf);
        return -1;
    }
    
    fwrite(dtree_buf, 1, dtree_len-0x3FDC, fp);
    fflush(fp);
    fclose(fp);
    
    free(dtree_buf);
    return 0;
}
