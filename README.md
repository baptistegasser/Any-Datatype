<!--
 _____________
< Hello there >
 -------------
  (\__/)||
_ (•ㅅ•)||
 \/o   \っ
-->

![Life Cycle](https://img.shields.io/badge/Life%20Cycle-Experimental-red)
![Unreal Engine Version](https://img.shields.io/badge/Unreal%20Engine%20Version-4.27-brightgreen)

# Any Datatype plugin
An Unreal Engine plugin providing a std like any data type allowing to store any value.

## Motivation
Unreal Engine currently does not provide an Any type and C++17 is not supported out of the box.
Even if C++17 was supported, the `std::any` would not be appropriate as it's not designed for the engine thus lacking several features such as :
- Blueprint usage
- Replication other the network
- Getting information on the stored data for debugging
- Working with the UPROPERTY system

## Current state
This is still an experimental plugin under developement and lack a lot of features to
be considered a reliable Unreal Engine type on the pare of `TVariant` or `TArray`.

It has however succefully been used without problems to make a C++ Blackboard for the AI
of the game Ikolia made during the winter session of the DDJV _(link to project coming soon)_.

## Roadmap
General roadmap for the development, not sorted by priority.

- [ ] Clean and tidy up the code
- [ ] Complete the automated tests
- [ ] Add documentation
- [ ] Optimization
    - [ ] General : memory, speed...
    - [ ] Avoid dynamic allocations for small, `nothrow_move_constructible` objects
- [ ] Support for network replication
- [ ] Support for blueprint usage
- [ ] Support for the save system
- [ ] Support for the UPROPERTY & Garbage collection system
- [ ] Release to Marketplace

## License

Distributed under the MIT License. See [`LICENSE`](LICENSE) for more information.
