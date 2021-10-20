#include <windows.h>
#include <stdio.h>

/* 
This program loads BCrypt functions from the dll at runtime.
Therefore it is not necessary to link the bcrypt.dll at compile time 
*/

#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)

/* function prototypes */
typedef NTSTATUS (NTAPI *BCryptOpenAlgorithmProvider_)(BCRYPT_ALG_HANDLE, LPCWSTR, LPCWSTR, ULONG);
typedef NTSTATUS (NTAPI *BCryptCloseAlgorithmProvider_)(BCRYPT_ALG_HANDLE, ULONG);
typedef NTSTATUS (NTAPI *BCryptGetProperty_)(BCRYPT_HANDLE, LPCWSTR, PUCHAR, ULONG, ULONG *, ULONG);
typedef NTSTATUS (NTAPI *BCryptCreateHash_)(BCRYPT_ALG_HANDLE, BCRYPT_HASH_HANDLE *, PUCHAR, ULONG, PUCHAR, ULONG, ULONG);
typedef NTSTATUS (NTAPI *BCryptHashData_)(BCRYPT_HASH_HANDLE, PUCHAR, ULONG, ULONG);
typedef NTSTATUS (NTAPI *BCryptFinishHash_)(BCRYPT_HASH_HANDLE, PUCHAR, ULONG, ULONG);
typedef NTSTATUS (NTAPI *BCryptDestroyHash_)(BCRYPT_HASH_HANDLE);


BCryptOpenAlgorithmProvider_ BCryptOpenAlgorithmProvider__;
BCryptCloseAlgorithmProvider_ BCryptCloseAlgorithmProvider__;
BCryptGetProperty_ BCryptGetProperty__;
BCryptCreateHash_ BCryptCreateHash__;
BCryptHashData_ BCryptHashData__;
BCryptFinishHash_ BCryptFinishHash__;
BCryptDestroyHash_ BCryptDestroyHash__;
 
/* Load functions from dll */
int load_functions()
{
    HINSTANCE lib;

    BCryptOpenAlgorithmProvider__ = NULL;
    BCryptCloseAlgorithmProvider__ = NULL;
    BCryptGetProperty__ = NULL;
    BCryptCreateHash__ = NULL;
    BCryptHashData__ = NULL;
    BCryptFinishHash__ = NULL;
    BCryptDestroyHash__ = NULL;

    lib = LoadLibraryA("bcrypt.dll");
    if(lib == NULL)
    {
        printf("Could not open bcrypt.dll | %x\n", GetLastError());
        return 0;
    }

    BCryptOpenAlgorithmProvider__ = (BCryptOpenAlgorithmProvider_) GetProcAddress(lib, "BCryptOpenAlgorithmProvider");
    if(BCryptOpenAlgorithmProvider__ == NULL)
    {
        printf("Could not BCryptOpenAlgorithmProvider | %x\n", GetLastError());
        return 0;       
    }

    BCryptCloseAlgorithmProvider__ = (BCryptCloseAlgorithmProvider_) GetProcAddress(lib, "BCryptCloseAlgorithmProvider");
    if(BCryptCloseAlgorithmProvider__ == NULL)
    {
        printf("Could not BCryptCloseAlgorithmProvider | %x\n", GetLastError());
        return 0;       
    }
    
    BCryptGetProperty__ = (BCryptGetProperty_) GetProcAddress(lib, "BCryptGetProperty");
    if(BCryptGetProperty__ == NULL)
    {
        printf("Could not BCryptGetProperty | %x\n", GetLastError());
        return 0;       
    }

    BCryptCreateHash__ = (BCryptCreateHash_) GetProcAddress(lib, "BCryptCreateHash");
    if(BCryptCreateHash__ == NULL)
    {
        printf("Could not BCryptCreateHash | %x\n", GetLastError());
        return 0;       
    }

    BCryptHashData__ = (BCryptHashData_) GetProcAddress(lib, "BCryptHashData");
    if(BCryptHashData__ == NULL)
    {
        printf("Could not BCryptHashData | %x\n", GetLastError());
        return 0;       
    }

    BCryptFinishHash__ = (BCryptFinishHash_) GetProcAddress(lib, "BCryptFinishHash");
    if(BCryptFinishHash__ == NULL)
    {
        printf("Could not BCryptFinishHash | %x\n", GetLastError());
        return 0;       
    }

    BCryptDestroyHash__ = (BCryptDestroyHash_) GetProcAddress(lib, "BCryptDestroyHash");
    if(BCryptDestroyHash__ == NULL)
    {
        printf("Could not BCryptDestroyHash | %x\n", GetLastError());
        return 0;       
    }

    FreeLibrary(lib);
    return 1;
}

BCRYPT_ALG_HANDLE initialize_sha256_algorithm()
{
    NTSTATUS err;
    BCRYPT_ALG_HANDLE hAesAlg;

    if(!NT_SUCCESS(err = BCryptOpenAlgorithmProvider__(&hAesAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0)))
    {
        printf("Error 0x%lx return 0ed by BCryptOpenAlgorithmProvider\n", err);
        return NULL;
    }
    return hAesAlg;
}

void print_bytes(PBYTE buffer, unsigned long size)
{
    if(buffer == NULL) return;

    for(unsigned long i = 0;i < size;i++)
    {
        printf("%02x", buffer[i]);
    }
}

PBYTE sha256_sum(BCRYPT_ALG_HANDLE hAlg, FILE *fp)
{
    if(fp == NULL) return 0;

    NTSTATUS err;

    BCRYPT_HASH_HANDLE hHash = NULL;
    DWORD cbData;

    // Buffer to read the file
    BYTE f_buffer[1024];
    unsigned long bytes_read = 1;

    DWORD hash_object_length;
    PBYTE hash_object = NULL;

    DWORD hash_size;
    PBYTE hash = NULL;

    // Calculate hash object size
    if(!NT_SUCCESS(err = BCryptGetProperty__(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE) &hash_object_length, sizeof(hash_object_length), &cbData, 0)))
    {
        printf("Error 0x%lx returned by BCryptGetProperty\n", err);
        goto Cleanup;
    }

    // Allocate hash object
    hash_object = HeapAlloc(GetProcessHeap(), 0, hash_object_length);
    if(hash_object == NULL)
    {
        printf("Memory allocation failed\n");
        goto Cleanup;
    }

    //Calculate hash size
    if(!NT_SUCCESS(err = BCryptGetProperty__(hAlg, BCRYPT_HASH_LENGTH, (PBYTE) &hash_size, sizeof(hash_size), &cbData, 0)))
    {
        printf("Error 0x%lx returned by BCryptGetProperty\n", err);
        goto Cleanup;
    }

    hash = HeapAlloc(GetProcessHeap(), 0, hash_size);
    if(hash == NULL)
    {
        printf("Memory allocation failed\n");
        goto Cleanup;
    }

    // Create the hash object
    if(!NT_SUCCESS(err = BCryptCreateHash__(hAlg, &hHash, hash_object, hash_object_length, NULL, 0, 0)))
    {
        printf("Error 0x%lx returned by BCryptCreateHash\n", err);
        goto Cleanup;
    }

    // Read the file and create the hash | only read 1024 byte at the time to prevent large memory allocation
    while(bytes_read > 0)
    {
        bytes_read = fread(f_buffer, sizeof(BYTE), sizeof(f_buffer), fp);
        if(!NT_SUCCESS(err = BCryptHashData__(hHash, f_buffer, bytes_read, 0)))
        {
            printf("Error 0x%lx returned by BCryptHashData\n", err);
            goto Cleanup;
        }
    }

    // Finalize the hash
    if(!NT_SUCCESS(err = BCryptFinishHash__(hHash, hash, hash_size, 0)))
    {
        printf("Error 0x%lx returned by BCryptFinishHash\n", err);
        goto Cleanup;
    }

    Cleanup:

    if(hHash)
    {
        BCryptDestroyHash__(hHash);
    }

    if(hash_object)
    {
        HeapFree(GetProcessHeap(), 0, hash_object);
    }

    return hash;
}

int main(int argc, char *argv[])
{
    if(argc < 2){
        printf("No input file specified\n");
        return 0;
    }

    if(load_functions() == 0) return 0;

    FILE *fp = fopen(argv[1], "rb");
    if(fp == NULL){
        printf("File %s could not be opened\n", argv[1]);
        return 0;
    }

    BCRYPT_ALG_HANDLE sha256_alg = initialize_sha256_algorithm();
    if(sha256_alg == NULL){
        fclose(fp);
        return 0;
    }

    PBYTE hash = sha256_sum(sha256_alg, fp);
    fclose(fp);
    BCryptCloseAlgorithmProvider__(sha256_alg, 0);

    if(hash == NULL) return 0;

    print_bytes(hash, 32);

    HeapFree(GetProcessHeap(), 0, hash);

    return 0;
}