package java.util;

public abstract class AbstractCollection<E> implements Collection<E> {

	protected AbstractCollection() {
	}

	@Override
	public abstract Iterator<E> iterator();

	@Override
	public abstract int size();

	@Override
	public boolean isEmpty() {
		return this.size() == 0;
	}

	@Override
	public boolean contains(Object o) {
		for(E e : this) {
			if(Objects.equals(e, o)) {
				return true;
			}
		}
		return false;
	}

	@Override
	public Object[] toArray() {
		return null; // TODO use ArrayList
	}

	@Override
	public <T> T[] toArray(T[] a) {
		return null; // TODO
	}

	@Override
	public boolean add(E e) {
		throw new UnsupportedOperationException();
	}

	@Override
	public boolean remove(Object o) {
		Iterator<E> it = iterator();
		while(it.hasNext()) {
			Object e = it.next();
			if(Objects.equals(e, o)) {
				it.remove();
				return true;
			}
		}
		return false;
	}

	@Override
	public boolean containsAll(Collection<?> c) {
		for(Object e : c) {
			if(!contains(e)) {
				return false;
			}
		}
		return true;
	}

	@Override
	public boolean addAll(Collection<? extends E> c) {
		boolean changed = false;
		for(E e : c) {
			if(add(e)) {
				changed = true;
			}
		}
		return changed;
	}

	@Override
	public boolean removeAll(Collection<?> c) {
		if(c == null) {
			throw new NullPointerException();
		}
		boolean changed = false;
		Iterator<E> it = iterator();
		while(it.hasNext()) {
			if(c.contains(it.next())) {
				it.remove();
				changed = true;
			}
		}
		return changed;
	}

	public boolean retainAll(Collection<?> c) {
		if(c == null) {
			throw new NullPointerException();
		}
		boolean changed = false;
		Iterator<E> it = iterator();
		while(it.hasNext()) {
			if(!c.contains(it.next())) {
				it.remove();
				changed = true;
			}
		}
		return changed;
	}

	@Override
	public void clear() {
		Iterator<E> it = iterator();
		while(it.hasNext()) {
			it.next();
			it.remove();
		}
	}

	@Override
	public String toString() {
		return null; // TODO
	}

}