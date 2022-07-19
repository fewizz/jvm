package java.nio.charset;

public class CodingErrorAction {

	private final String name_;

	private CodingErrorAction(String name) {
		this.name_ = name;
	}

	public static final CodingErrorAction
		IGNORE  = new CodingErrorAction("IGNORE"),
		REPLACE = new CodingErrorAction("REPLACE"),
		REPORT  = new CodingErrorAction("REPORT");

	@Override
	public String toString() {
		return name_;
	}

}