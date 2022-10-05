package java.io;

public final class FileDescriptor {

	final int value_;

	public FileDescriptor() { // Why??
		this.value_ = -1;
	}

	FileDescriptor(int fd) {
		this.value_ = fd;
	}

	public boolean valid() {
		return this.value_ != -1;
	}

	public static native long sync(int fd);

	public void sync() throws SyncFailedException {
		if(sync(this.value_) == -1) {
			throw new SyncFailedException("Bad file descriptor");
		}
	}

	private static native int stderr_fd();
	private static native int stdin_fd();
	private static native int stdout_fd();

	public static final FileDescriptor
		err = new FileDescriptor(stderr_fd()),
		in  = new FileDescriptor(stdin_fd()),
		out = new FileDescriptor(stdout_fd());

}