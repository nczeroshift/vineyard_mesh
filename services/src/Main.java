import org.nczeroshift.vineyard.VYGateway;
import org.nczeroshift.vineyard.VYNode;
import org.nczeroshift.vineyard.VYPacket;
import org.nczeroshift.vineyard.payloads.VYPayloadChannel;

import java.util.List;

public class Main {

    public static void waitFor(long time){
        try{
            Thread.sleep(time);
        }catch (Exception e){

        }
    }

    public static void main(String[] args) throws Exception {
        String addr = "http://192.168.137.8/";
        VYGateway gateway = new VYGateway(addr);
        List<VYNode> nodes = gateway.getNodes();

        for (VYNode node : nodes) {
            System.out.println(node);

            sendPacketAndReceive(gateway, node, new VYPacket(VYPacket.Type.VOLTAGE).setPayload(new VYPayloadChannel((byte) 0)));
            sendPacketAndReceive(gateway, node, new VYPacket(VYPacket.Type.TEMPERATURE));
            sendPacketAndReceive(gateway, node, new VYPacket(VYPacket.Type.HUMIDITY));
        }
    }

    private static void sendPacketAndReceive(VYGateway gateway, VYNode node, VYPacket pk) throws Exception {
        int tryCount = 20;
        while(tryCount -- > 0) {
            if(gateway.sendPacket(node, pk)) {
                waitFor(100);
                VYPacket packet = gateway.readPacket(node, pk);

                if(packet != null){
                    // Success!
                    System.out.println(packet.toString());
                    tryCount = 0;
                } else
                    waitFor(1000);
            } else
                waitFor(1000);
        }
    }
}
