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

	public abstract CharsetDecoder newDecoder();

	public abstract CharsetEncoder newEncoder();

	public boolean canEncode() {
		return true;
	}

	@Override
	public int hashCode() {
		return name_.hashCode();
	}

	@Override
	public boolean equals(Object obj) {
		if(obj instanceof Charset) {
			if(obj == this) {
				return true;
			}
			return this.name_.equals(obj);
		}
		return false;
	}

	@Override
	public String toString() {
		return name_.toString();
	}

}
