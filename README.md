This software places people in tables such that they are next to their preferable companions. To do this it borrows the Ising model from statistical physics and uses simulated annealing. More of these in Wikipedia:
https://en.wikipedia.org/wiki/Ising_model
https://en.wikipedia.org/wiki/Simulated_annealing

The software has no error handling of any kind and is done as a proof of concept. Thus any malformed input will break it.

Useful further development would include correctly placing couples according to social norms, i.e. ensuring the lady on the right side of a gentleman or the married couples seated opposite to each other. Also, right now the program ignores gender. The user can take this into account by setting the same sex people to "dislike" each other if they should not be seated next to each other. Also useful would be to have a graphical user interface.
