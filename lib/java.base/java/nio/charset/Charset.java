package java.nio.charset;

public abstract class Charset implements Comparable<Charset> {

	private final String name_;
	private final String[] aliases_;

	protected Charset(String canonicalName, String[] aliases) {
		this.name_ = canonicalName;
		this.aliases_ = aliases;
	}

	public final String name() {
		return name_;
	}

	public String displayName() {
		return name();
	}

	public final boolean isRegistered() {
		return !(name_.startsWith("X-") || name_.startsWith("x-"));
	}

	public boolean canEncode() {
		return true;
	}

}
