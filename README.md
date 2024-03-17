libsnark merkle circuit example

The example shows how to generate proof for one merkle path on one merkle tree with depth 3.

1/ compile
 ```
 mkdir build; cd build; cmake ..; make
 ```
 You can find the "merkle" binary under the merkle folder.

2/ setup
```
./merkle setup
```

3/ prove
```
./merkle prove
```
Record down the root information, which is used on verify.

4/ verify
```
./merkle [root]
```
