//----
// msmmap
//----

//-----------------------------------------------------------------------------
// -- standard cpp lib includes -- 
//-----------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <map>

//-----------------------------------------------------------------------------
// -- standard c lib includes -- 
//-----------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _WIN32
    #include "Windows.h"
#else
    #include <sys/mman.h>
    #include <unistd.h>
#endif


class MMap
{
  public:
      MMap();
      ~MMap();

      //----------------------------------------------------------------------
      void  open(const std::string &path,
                 int data_size);

      //----------------------------------------------------------------------
      void  close();

      //----------------------------------------------------------------------
      void *data_ptr() const
          { return m_data; }

  private:
      void      *m_data;
      int        m_data_size;

#if !defined(_WIN32)
      // memory-map file descriptor
      int       m_mmap_fd;
#else
      // handles for windows mmap
      HANDLE    m_file_hnd;
      HANDLE    m_map_hnd;
#endif

};

//-----------------------------------------------------------------------------
MMap::MMap()
: m_data(NULL),
  m_data_size(0),
#if !defined(_WIN32)
  m_mmap_fd(-1)
#else
  // windows
  m_file_hnd(INVALID_HANDLE_VALUE),
  m_map_hnd(INVALID_HANDLE_VALUE)
#endif
{
    // empty
}

//-----------------------------------------------------------------------------
MMap::~MMap()
{
    close();
}

//-----------------------------------------------------------------------------
void
MMap::open(const std::string &path,
           int data_size)
{
    if(m_data != NULL)
    {
        std::cout << "ERROR!" << std::endl;
        return;
    }

#if !defined(_WIN32)
    m_mmap_fd   = ::open(path.c_str(),
                         (O_RDWR | O_CREAT),
                         (S_IRUSR | S_IWUSR));

    m_data_size = data_size;

    if (m_mmap_fd == -1) 
        std::cout << "ERROR!" << std::endl;

    m_data = ::mmap(0,
                    m_data_size,
                    (PROT_READ | PROT_WRITE),
                    MAP_SHARED,
                    m_mmap_fd, 0);

    if (m_data == MAP_FAILED) 
    {
        std::cout << "ERROR!" << std::endl;
        return;
    }
#else
    m_file_hnd = CreateFile(path.c_str(),
                            (GENERIC_READ | GENERIC_WRITE),
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_RANDOM_ACCESS,
                            NULL);

    if (m_file_hnd == INVALID_HANDLE_VALUE)
    {
        std::cout << "ERROR!" << std::endl;
        return;
    }

    m_map_hnd = CreateFileMapping(m_file_hnd,
                                  NULL,
                                  PAGE_READWRITE,
                                  0, 0, 0);

    if (m_map_hnd == NULL)
    {
        CloseHandle(m_file_hnd);
        std::cout << "ERROR!" << std::endl;
        return;
    }

    m_data = MapViewOfFile(m_map_hnd,
                           FILE_MAP_ALL_ACCESS,
                           0, 0, 0);

    if (m_data == NULL)
    {
        CloseHandle(m_map_hnd);
        CloseHandle(m_file_hnd);
        std::cout << "ERROR!" << std::endl;
        return;
    }
#endif
}

//-----------------------------------------------------------------------------
void
MMap::close()
{
    // simple return if the mmap isn't active
    if(m_data == NULL)
        return;
    
#if !defined(_WIN32)
    
    if(munmap(m_data, m_data_size) == -1) 
    {
        std::cout << "ERROR!" << std::endl;
        return;
    }
    
    if(::close(m_mmap_fd) == -1)
    {
        std::cout << "ERROR!" << std::endl;
        return;
    }

    m_mmap_fd   = -1;

#else
    UnmapViewOfFile(m_data);
    CloseHandle(m_map_hnd);
    CloseHandle(m_file_hnd);
    m_file_hnd = INVALID_HANDLE_VALUE;
    m_map_hnd  = INVALID_HANDLE_VALUE;
#endif

    // clear data pointer and size member
    m_data      = NULL;
    m_data_size = 0;

}


//-----------------------------------------------------------------------------
int main()
{
    char vals[4] = {1,2,3,4};
    
    std::ofstream ofs;
    ofs.open("tout_example.bin");
    if(!ofs.is_open())
    {
        // #ERROR!
        return -1;
    }
    ofs.write(vals,4);
    ofs.close();
    
    MMap m;
    m.open("tout_example.bin",4);
    char *res_vals = (char*)m.data_ptr();

    std::cout  << (int)res_vals[0] <<  " "
               << (int)res_vals[1] <<  " "
               << (int)res_vals[2] <<  " "
               << (int)res_vals[3] <<  std::endl;
    
    res_vals[2] = 20;
    m.close();
    
    
    m.open("tout_example.bin",4);
    res_vals = (char*)m.data_ptr();
    std::cout  << (int)res_vals[0] <<  " "
               << (int)res_vals[1] <<  " "
               << (int)res_vals[2] <<  " "
               << (int)res_vals[3] <<  std::endl;

}




