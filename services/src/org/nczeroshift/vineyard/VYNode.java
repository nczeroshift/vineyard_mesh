package org.nczeroshift.vineyard;

public class VYNode {
    public VYNode(Integer id, Integer address){
        nodeId = id;
        nodeAddress = address;
    }

    private Integer nodeId;
    private Integer nodeAddress;

    public Integer getId(){
        return nodeId;
    }

    public Integer getAddress(){
        return nodeAddress;
    }

    @Override
    public String toString() {
        return "VYNode{" +
                "nodeId=" + nodeId +
                ", nodeAddress=" + nodeAddress +
                '}';
    }
}
