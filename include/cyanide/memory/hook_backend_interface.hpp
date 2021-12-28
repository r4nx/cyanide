#ifndef CYANIDE_MEMORY_HOOK_BACKEND_INTERFACE_HPP_
#define CYANIDE_MEMORY_HOOK_BACKEND_INTERFACE_HPP_

/**
 * @brief The backend interface you need to implement in order to perform
 * hooking.
 *
 * You should manually save the context (like a hook object of your backend
 * library), as uninstall() is called without any parameters.
 */
class DetourBackendInterface {
public:
    virtual ~DetourBackendInterface() = default;

    virtual void  install(void *source, const void *destination) = 0;
    virtual void  uninstall()                                    = 0;
    virtual void *get_trampoline()                               = 0;
};

#endif // !CYANIDE_MEMORY_HOOK_BACKEND_INTERFACE_HPP_
