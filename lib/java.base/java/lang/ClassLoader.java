package java.lang;

import jvm.AppClassLoader;

public abstract class ClassLoader {
	private final ClassLoader parent_;
	private final String name_;

	protected ClassLoader(String name, ClassLoader parent) {
		this.parent_ = parent;
		this.name_ = name;
	}

	protected ClassLoader(ClassLoader parent) {
		this(null, parent);
	}

	protected ClassLoader() {
		this(null, getSystemClassLoader());
	}

	public final ClassLoader getParent() {
		return this.parent_;
	}

	public String getName() {
		return this.name_;
	}

	protected final native Class<?> defineClass(
		String name,
		byte[] b,
		int off,
		int len
	) throws ClassFormatError;

	public Class<?> loadClass(String name) throws ClassNotFoundException {
		/* Invoking this method is equivalent to invoking
		   loadClass(name, false). */
		return this.loadClass(name, false);
	}

	private static native Class<?> loadClassJVM(String name);

	/* Loads the class with the specified binary name. The default
	   implementation of this method searches for classes in the following
	   order: */
	protected Class<?> loadClass(
		String name,
		boolean resolve
	) throws ClassNotFoundException {
		/* 1. Invoke findLoadedClass(String) to check if the class has already
		      been loaded. */
		Class<?> c = this.findLoadedClass(name);

		if(c == null) {
			/* 2. Invoke the loadClass method on the parent class loader. If the
			      parent is null the class loader built into the virtual machine
			      is used, instead. */
			try {
				c = parent_ != null ?
					parent_.loadClass(name) :
					loadClassJVM(name);
			} catch(ClassNotFoundException e) {
				// Nope
			}
		}

		/* 3. Invoke the findClass(String) method to find the class. */
		if(c == null) c = this.findClass(name);

		/* Throws: ClassNotFoundException - If the class could not be found */
		if(c == null) throw new ClassNotFoundException(name);

		/* If the class was found using the above steps, and the resolve flag is
		   true, this method will then invoke the resolveClass(Class) method on
		   the resulting Class object. */
		if(resolve) {
			resolveClass(c);
		}

		return c;
	}

	protected Class<?> findClass(String name) throws ClassNotFoundException {
		/* The default implementation throws ClassNotFoundException. */
		throw new ClassNotFoundException();
	}

	protected final void resolveClass(Class<?> c) {
		// do nothin
	}

	protected final native Class<?> findLoadedClass(String name);

	public static ClassLoader getSystemClassLoader() {
		return AppClassLoader.INSTANCE;
	}
}