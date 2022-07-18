package java.nio.charset;

public abstract class Charset {

	private final String canonincalName_;
	private final String[] aliases_;

	protected Charset(String canonicalName, String[] aliases) {
		this.canonincalName_ = canonicalName;
		this.aliases_ = aliases;
	}

	public final String name() {
		return canonincalName_;
	}

	public String displayName() {
		return name();
	}

}
