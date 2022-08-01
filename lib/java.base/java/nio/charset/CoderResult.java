package java.nio.charset;

public class CoderResult {
	public static final CoderResult OVERFLOW  = new CoderResult() {

		@Override
		public boolean isError() {
			return true;
		}

	};

	public static final CoderResult UNDERFLOW = new CoderResult() {

		@Override
		public boolean isError() {
			return true;
		}

	};

	public boolean isError() {
		return false;
	}

}