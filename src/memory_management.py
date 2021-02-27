import resource
"""
Sets maximal virtual memory to <max_memory> MB
"""
def limit_virtual_memory(max_memory):
    MAX_VIRTUAL_MEMORY = max_memory * 1024 * 1024
    # The tuple below is of the form (soft limit, hard limit). Limit only
    # the soft part so that the limit can be increased later (setting also
    # the hard limit would prevent that).
    # When the limit cannot be changed, setrlimit() raises ValueError.
    resource.setrlimit(resource.RLIMIT_AS, (MAX_VIRTUAL_MEMORY, resource.RLIM_INFINITY))