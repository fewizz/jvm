package java.io;

public final class FileDescriptor {

	final long value_;

	public FileDescriptor() { // Why??
		this.value_ = (long) -1;
	}

	FileDescriptor(long fd) {
		this.value_ = fd;
	}

	public boolean valid() {
		return this.value_ != -1;
	}

	public static native long sync(long fd);

	public void sync() throws SyncFailedException {
		if(sync(this.value_) == -1) {
			throw new SyncFailedException("Bad file descriptor");
		}
	}

	private static native long stderr_fd();
	private static native long stdin_fd();
	private static native long stdout_fd();

	static final FileDescriptor
		err = new FileDescriptor(stderr_fd()),
		in  = new FileDescriptor(stdin_fd()),
		out = new FileDescriptor(stdout_fd());

}