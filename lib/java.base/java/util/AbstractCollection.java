package java.util;

public abstract class AbstractCollection<E> implements Collection<E> {

	protected AbstractCollection() {
	}

	@Override
	public abstract Iterator<E> iterator();

	@Override
	public boolean isEmpty() {
		return this.size() == 0;
	}

	/*@Override
	public boolean contains(Object o) {
		for(E e : this) {
			if(Objects.equals(e, o)) {
				return true;
			}
		}
		return false;
	}*/

	@Override
	public Object[] toArray() {
		int len = this.size();

		Object[] array = new Object[len];

		Iterator<E> it = iterator();

		for(int x = 0; x < len; ++x) {
			array[x] = it.next();
		}

		return array;
	}

}