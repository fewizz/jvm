package java.lang;

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
		this(null, null);
	}

	public String getName() { return this.name_; }
}