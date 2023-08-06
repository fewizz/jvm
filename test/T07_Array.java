public class T07_Array {

	public static void main(String... args) {
		int arr[] = new int[128];
		if(arr.length != 128) {
			System.exit(1);
		}

		for(int x = 0; x < arr.length; ++x) {
			if(arr[x] != 0) {
				System.exit(2);
			}
		}

		for(int x = 0; x < arr.length; ++x) {
			arr[x] = x;
		}

		for(int x = 0; x < arr.length; ++x) {
			if(arr[x] != x) {
				System.exit(3);
			}
		}

		boolean outOfBounds = false;

		{
			try {
				arr[127] = 42;
			}
			catch(IndexOutOfBoundsException e) {
				outOfBounds = true;
			}
			if(outOfBounds) {
				System.exit(4);
			}
		}

		{
			outOfBounds = false;
			try {
				arr[128] = 42;
			}
			catch(IndexOutOfBoundsException e) {
				outOfBounds = true;
			}
			if(!outOfBounds) {
				System.exit(5);
			}
		}

		{
			outOfBounds = false;
			try {
				arr[-1] = 42;
			}
			catch(IndexOutOfBoundsException e) {
				outOfBounds = true;
			}
			if(!outOfBounds) {
				System.exit(6);
			}
		}
	}

}