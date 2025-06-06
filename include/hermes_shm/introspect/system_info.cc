#define HSHM_COMPILING_DLL
#define __HSHM_IS_COMPILING__

#include "hermes_shm/introspect/system_info.h"

#include <dlfcn.h>

#include <cstdlib>

#include "hermes_shm/constants/macros.h"
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
// LINUX
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#if __linux__
#include <sys/sysinfo.h>
#else
#include <sys/sysctl.h>
#endif
#include <sys/types.h>
#include <unistd.h>
// WINDOWS
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
#include <windows.h>
#else
#error \
    "Must define either HSHM_ENABLE_PROCFS_SYSINFO or HSHM_ENABLE_WINDOWS_SYSINFO"
#endif

namespace hshm {

void SystemInfo::RefreshCpuFreqKhz() {
#ifdef HSHM_IS_HOST
  for (int i = 0; i < ncpu_; ++i) {
    cur_cpu_freq_[i] = GetCpuFreqKhz(i);
  }
#endif
}

size_t SystemInfo::GetCpuFreqKhz(int cpu) {
#ifdef HSHM_IS_HOST
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  // Read /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq
  std::string cpu_str = hshm::Formatter::format(
      "/sys/devices/system/cpu/cpu{}/cpufreq/cpuinfo_cur_freq", cpu);
  std::ifstream cpu_file(cpu_str);
  size_t freq_khz;
  cpu_file >> freq_khz;
  return freq_khz;
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return 0;
#endif
#else
  return 0;
#endif
}

size_t SystemInfo::GetCpuMaxFreqKhz(int cpu) {
#ifdef HSHM_IS_HOST
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  // Read /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq
  std::string cpu_str = hshm::Formatter::format(
      "/sys/devices/system/cpu/cpu{}/cpufreq/cpuinfo_max_freq", cpu);
  std::ifstream cpu_file(cpu_str);
  size_t freq_khz;
  cpu_file >> freq_khz;
  return freq_khz;
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return 0;
#endif
#else
  return 0;
#endif
}

size_t SystemInfo::GetCpuMinFreqKhz(int cpu) {
#ifdef HSHM_IS_HOST
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  // Read /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_cur_freq
  std::string cpu_str = hshm::Formatter::format(
      "/sys/devices/system/cpu/cpu{}/cpufreq/cpuinfo_min_freq", cpu);
  std::ifstream cpu_file(cpu_str);
  size_t freq_khz;
  cpu_file >> freq_khz;
  return freq_khz;
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return 0;
#endif
#else
  return 0;
#endif
}

size_t SystemInfo::GetCpuMinFreqMhz(int cpu) {
  return GetCpuMinFreqKhz(cpu) / 1000;
}

size_t SystemInfo::GetCpuMaxFreqMhz(int cpu) {
  return GetCpuMaxFreqKhz(cpu) / 1000;
}

void SystemInfo::SetCpuFreqMhz(int cpu, size_t cpu_freq_mhz) {
  SetCpuFreqKhz(cpu, cpu_freq_mhz * 1000);
}

void SystemInfo::SetCpuFreqKhz(int cpu, size_t cpu_freq_khz) {
  SetCpuMinFreqKhz(cpu, cpu_freq_khz);
  SetCpuMaxFreqKhz(cpu, cpu_freq_khz);
}

void SystemInfo::SetCpuMinFreqKhz(int cpu, size_t cpu_freq_khz) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  std::string cpu_str = hshm::Formatter::format(
      "/sys/devices/system/cpu/cpu{}/cpufreq/scaling_min_freq", cpu);
  std::ofstream min_freq_file(cpu_str);
  min_freq_file << cpu_freq_khz;
#endif
}

void SystemInfo::SetCpuMaxFreqKhz(int cpu, size_t cpu_freq_khz) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  std::string cpu_str = hshm::Formatter::format(
      "/sys/devices/system/cpu/cpu{}/cpufreq/scaling_max_freq", cpu);
  std::ofstream max_freq_file(cpu_str);
  max_freq_file << cpu_freq_khz;
#endif
}

int SystemInfo::GetCpuCount() {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)

#if __linux__
  return get_nprocs_conf();
#else
  int count;
  using size_t = std::size_t;
  size_t count_len = sizeof(count);
#if __APPLE__
  if (sysctlbyname("hw.physicalcpu", &count, &count_len, NULL, 0) == -1) {
#else
  int mib[2];
  if (sysctl(mib, 2, &count, &count_len, NULL, 0) == -1) {
#endif
    perror("sysctl");
    return 1;
  }
  return count;
#endif

#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);
  return sys_info.dwNumberOfProcessors;

#endif
}

int SystemInfo::GetPageSize() {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  return getpagesize();
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  SYSTEM_INFO sys_info;
  GetSystemInfo(&sys_info);
  if (sys_info.dwAllocationGranularity != 0) {
    return sys_info.dwAllocationGranularity;
  }
  return sys_info.dwPageSize;
#endif
}

int SystemInfo::GetTid() {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
#ifdef SYS_gettid
#ifdef __linux__
  return (pid_t)syscall(SYS_gettid);
#else
  return GetPid();
#endif
#else
#warning "GetTid is not defined"
  return GetPid();
#endif
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return GetCurrentThreadId();
#endif
}

int SystemInfo::GetPid() {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
#ifdef SYS_getpid
#ifdef __OpenBSD__
  return (pid_t)getpid();
#else
  return (pid_t)syscall(SYS_getpid);
#endif
#else
#warning "GetPid is not defined"
  return 0;
#endif
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return GetCurrentProcessId();
#endif
}

int SystemInfo::GetUid() {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  return getuid();
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return 0;
#endif
};

int SystemInfo::GetGid() {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  return getgid();
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return 0;
#endif
};

size_t SystemInfo::GetRamCapacity() {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
#if __APPLE__ || __OpenBSD__
  int mib[2];
  uint64_t mem_total;  // Use uint64_t for memory sizes

  mib[0] = CTL_HW;
#if __APPLE__
  mib[1] = HW_MEMSIZE;  // This is what you're looking for
#else
  mib[1] = HW_PHYSMEM;
#endif
  using size_t = std::size_t;
  size_t len = sizeof(mem_total);
  if (sysctl(mib, 2, &mem_total, &len, NULL, 0) == -1) {
    perror("sysctl");
    return 1;
  } else {
    return mem_total;
  }
#else
  struct sysinfo info;
  sysinfo(&info);
  return info.totalram;
#endif
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  MEMORYSTATUSEX mem_info;
  mem_info.dwLength = sizeof(mem_info);
  GlobalMemoryStatusEx(&mem_info);
  return (size_t)mem_info.ullTotalPhys;
#endif
}

void SystemInfo::YieldThread() {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  sched_yield();
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  Yield();
#endif
}

bool SystemInfo::CreateTls(ThreadLocalKey &key, void *data) {
#ifdef HSHM_ENABLE_PROCFS_SYSINFO
  key.pthread_key_ = pthread_key_create(&key.pthread_key_, nullptr);
  return key.pthread_key_ == 0;
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  key.windows_key_ = TlsAlloc();
  if (key.windows_key_ == TLS_OUT_OF_INDEXES) {
    return false;
  }
  return TlsSetValue(key.windows_key_, data);
#endif
}

bool SystemInfo::SetTls(const ThreadLocalKey &key, void *data) {
#ifdef HSHM_ENABLE_PROCFS_SYSINFO
  return pthread_setspecific(key.pthread_key_, data) == 0;
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return TlsSetValue(key.windows_key_, data);
#endif
}

void *SystemInfo::GetTls(const ThreadLocalKey &key) {
#ifdef HSHM_ENABLE_PROCFS_SYSINFO
  return pthread_getspecific(key.pthread_key_);
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return TlsGetValue(key.windows_key_);
#endif
}

bool SystemInfo::CreateNewSharedMemory(File &fd, const std::string &name,
                                       size_t size) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  fd.posix_fd_ = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
  if (fd.posix_fd_ < 0) {
    return false;
  }
  int ret = ftruncate(fd.posix_fd_, size);
  if (ret < 0) {
    close(fd.posix_fd_);
    return false;
  }
  return true;
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  fd.windows_fd_ =
      CreateFileMapping(INVALID_HANDLE_VALUE,  // use paging file
                        nullptr,               // default security
                        PAGE_READWRITE,        // read/write access
                        0,         // maximum object size (high-order DWORD)
                        size,      // maximum object size (low-order DWORD)
                        nullptr);  // name of mapping object
  return fd.windows_fd_ != nullptr;
#endif
}

bool SystemInfo::OpenSharedMemory(File &fd, const std::string &name) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  fd.posix_fd_ = shm_open(name.c_str(), O_RDWR, 0666);
  return fd.posix_fd_ >= 0;
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  fd.windows_fd_ = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name.c_str());
  return fd.windows_fd_ != nullptr;
#endif
}

void SystemInfo::CloseSharedMemory(File &file) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  close(file.posix_fd_);
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  CloseHandle(file.windows_fd_);
#endif
}

void SystemInfo::DestroySharedMemory(const std::string &name) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  shm_unlink(name.c_str());
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
#endif
}

void *SystemInfo::MapPrivateMemory(size_t size) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
#if __APPLE__ || __OpenBSD__
  return mmap(nullptr, size, PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#else
  return mmap64(nullptr, size, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
#endif
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return VirtualAlloc(nullptr, size, MEM_COMMIT, PAGE_READWRITE);
#endif
}

void *SystemInfo::MapSharedMemory(const File &fd, size_t size, i64 off) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  void *ptr = mmap64(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED,
                     fd.posix_fd_, off);
  if (ptr == MAP_FAILED) {
    perror("mmap");
    return nullptr;
  }
  return ptr;
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  // Convert i64 to low and high dwords
  DWORD highDword = (DWORD)((off >> 32) & 0xFFFFFFFF);
  DWORD lowDword = (DWORD)(off & 0xFFFFFFFF);
  void *ret = MapViewOfFile(fd.windows_fd_,       // handle to map object
                            FILE_MAP_ALL_ACCESS,  // read/write permission
                            highDword,            // file offset high
                            lowDword,             // file offset low
                            size);                // number of bytes to map
  if (ret == nullptr) {
    DWORD error = GetLastError();
    LPVOID msg_buf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPTSTR)&msg_buf, 0, NULL);
    printf("MapViewOfFile failed with error: %s\n", (char *)msg_buf);
    LocalFree(msg_buf);
  }
  return ret;
#endif
}

void SystemInfo::UnmapMemory(void *ptr, size_t size) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  munmap(ptr, size);
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  VirtualFree(ptr, size, MEM_RELEASE);
#endif
}

void *SystemInfo::AlignedAlloc(size_t alignment, size_t size) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  return aligned_alloc(alignment, size);
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return _aligned_malloc(size, alignment);
#endif
}

std::string SystemInfo::Getenv(const char *name, size_t max_size) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  char *var = getenv(name);
  if (var == nullptr) {
    return "";
  }
  return std::string(var);
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  std::string var;
  var.resize(max_size);
  GetEnvironmentVariable(name, var.data(), var.size());
  return var;
#endif
  std::cout << "undefined" << std::endl;
  return "";
}

void SystemInfo::Setenv(const char *name, const std::string &value,
                        int overwrite) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  setenv(name, value.c_str(), overwrite);
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  SetEnvironmentVariable(name, value.c_str());
#endif
}

void SystemInfo::Unsetenv(const char *name) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  unsetenv(name);
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  SetEnvironmentVariable(name, nullptr);
#endif
}

SharedLibrary::SharedLibrary(const std::string &name) : handle_(nullptr) {
  Load(name);
}

SharedLibrary::~SharedLibrary() {
  if (handle_) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
    dlclose(handle_);
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
    ::FreeLibrary((HMODULE)handle_);
#endif
    handle_ = nullptr;
  }
}

void SharedLibrary::Load(const std::string &name) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  handle_ = dlopen(name.c_str(), RTLD_GLOBAL | RTLD_NOW);
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  handle_ = LoadLibraryA(name.c_str());
#endif
}

std::string SharedLibrary::GetError() const {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  return std::string(dlerror());
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return std::string();
#endif
}

void *SharedLibrary::GetSymbol(const std::string &name) {
#if defined(HSHM_ENABLE_PROCFS_SYSINFO)
  return dlsym(handle_, name.c_str());
#elif defined(HSHM_ENABLE_WINDOWS_SYSINFO)
  return (void *)::GetProcAddress((HMODULE)handle_, name.c_str());
#endif
}

SharedLibrary::SharedLibrary(SharedLibrary &&other) noexcept
    : handle_(other.handle_) {
  other.handle_ = nullptr;
}

SharedLibrary &SharedLibrary::operator=(SharedLibrary &&other) noexcept {
  if (this != &other) {
    handle_ = other.handle_;
    other.handle_ = nullptr;
  }
  return *this;
}

}  // namespace hshm
