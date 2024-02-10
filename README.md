# rwDriver

A kernel-mode driver which does a simple read/write process memory with `NtQueryCompositionSurfaceStatistics`.

## How to use

1. Build the driver.
2. Map the driver. (kdmapper is recommended, as it is not signed)
3. You're done.

Follow the guide:

```c++
#define MAGIC 0x57616E654C6F6C69

typedef enum _USERMODE_COMMAND_MODE {
	Read,
	Write
} USERMODE_COMMAND_MODE;

typedef struct _USERMODE_COMMAND {
	unsigned long long magic = MAGIC;
	USERMODE_COMMAND_MODE mode;
	void* arg;
	void* retrn;
} USERMODE_COMMAND, * PUSERMODE_COMMAND;

typedef struct _READ_CMD_ARGS {
	DWORD pid;
	uintptr_t wh;
	size_t sz;
} READ_CMD_ARGS, * PREAD_CMD_ARGS;

typedef struct _WRITE_CMD_ARGS {
	DWORD pid;
	uintptr_t wh;
	void* what;
	size_t sz;
} WRITE_CMD_ARGS, * PWRITE_CMD_ARGS;
```

It is the define of args. You should define the same structures in your program, and execute the hook function.

The hook function is defined like this:

```c++
NTSTATUS HookFunction(
	HANDLE a1,
	uintptr_t a2
) {
	auto cvrt = (PUSERMODE_COMMAND)a1;
	if (!MmIsAddressValid(a1) || cvrt->magic != MAGIC) {
		return ORIGINAL;
	}

	if (cvrt->mode == Read) { // Return: Read Data
		return READ;
	}
	else if (cvrt->mode == Write) { // No Return
		return WRITE;
	}
	
	return STATUS_SUCCESS;
}
```

You can see, you should use first argument as the structure. Happy use!