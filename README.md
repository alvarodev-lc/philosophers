# Philosophers dining problem

<img height="500px" width="950px" src="https://cdn.thecollector.com/wp-content/uploads/2020/12/greek-philosophers-presocratics-heraclitus-democritus-pickenoy.jpg">

<strong><i>The dining philosophers problem</i></strong> is an example problem often used in concurrent algorithm design to illustrate synchronization issues and techniques for resolving them.

# Problem introduction:
<img src="https://pages.mtu.edu/~shene/NSF-3/e-Book/MUTEX/DIAGRAM-philosopher-lefty-has-1.jpg">
The Dining Philosopher Problem states that N philosophers seated around a circular table with a large bowl of spaghetti and one fork between each pair of philosopher. Each philosopher is doing one of the three things: eating, sleeping, thinking. While eating, they are not thinking or sleeping, while sleeping, they are not eating or thinking and of course, while thinking, they are not eating or sleeping. I should state that philosophers aren’t aware of other philosophers status.
