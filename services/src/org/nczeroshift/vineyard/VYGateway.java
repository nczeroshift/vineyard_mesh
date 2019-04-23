package org.nczeroshift.vineyard;

import org.apache.commons.io.IOUtils;
import org.apache.http.HttpEntity;
import org.apache.http.HttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.entity.ContentType;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.CloseableHttpClient;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.protocol.HttpProcessorBuilder;

import java.io.InputStream;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.List;

/**
 * VY(Vineyard) NRF24 based gateway
 */
public class VYGateway {
    String networkAddress;
    List<VYNode> nodesList = new ArrayList<>();

    public VYGateway(String address){
        networkAddress = address;
    }

    public List<VYNode> getNodes() throws Exception {
        fetchNodesList();
        return nodesList;
    }

    private void fetchNodesList() throws Exception {
        List<VYNode> ret = new ArrayList<VYNode>();

        HttpGet httpGet = new HttpGet(networkAddress+"?list");

        CloseableHttpClient httpclient = HttpClients.custom()
                .setHttpProcessor(HttpProcessorBuilder.create().build())
                .build();

        HttpResponse response = httpclient.execute(httpGet);
        HttpEntity entity = response.getEntity();

        if (entity != null && response.getStatusLine().getStatusCode() == 200) {
            try (InputStream instream = entity.getContent()) {
                String data = IOUtils.toString(instream, Charset.forName("ASCII"));
                String [] lines = data.split("\n");
                for(int i = 1;i<lines.length;i++){
                    String row[] = lines[i].split(",");
                    String id = row[0];
                    String address = row[1];
                    ret.add(new VYNode(Integer.parseInt(id),Integer.parseInt(address)));
                }
            }
        }
        else
            throw new Exception("Unable to fetch nodes list");

        nodesList = ret;
    }

    public boolean sendPacket(VYNode node, VYPacket packet) throws Exception{
        HttpPost httppost = new HttpPost(networkAddress+"?send");

        CloseableHttpClient httpclient = HttpClients.custom()
                .setHttpProcessor(HttpProcessorBuilder.create().build())
                .build();

        String payload = "id="+node.getAddress(); // node id
        payload += "&tp="+(char)packet.getType().code; // packet type

        if(packet.getPayload() == null)
            payload         +=  "&dt=00";
        else
            payload += "&dt="+packet.getPayload().stream();

        payload += "\r\r";

        StringEntity se = new StringEntity(payload, ContentType.TEXT_PLAIN);
        httppost.setHeader("Content-Type","plain/text");
        httppost.setHeader("Content-Length",String.valueOf(payload.length()));
        httppost.setEntity(se);

        HttpResponse response = httpclient.execute(httppost);

        return response.getStatusLine().getStatusCode() == 200;
    }

    public int getFreeRam(){
return 0;
    }

    public VYPacket readPacket(VYNode node, VYPacket packet) throws Exception{
        HttpGet httpPost = new HttpGet(networkAddress+"?read");

        CloseableHttpClient httpclient = HttpClients.custom()
                .setHttpProcessor(HttpProcessorBuilder.create().build())
                .build();

        httpPost.setHeader("Content-Type","plain/text");

        HttpResponse response = httpclient.execute(httpPost);
        HttpEntity entity = response.getEntity();

        if (entity != null && response.getStatusLine().getStatusCode() == 200) {
            try (InputStream instream = entity.getContent()) {
                String data = IOUtils.toString(instream, Charset.forName("ASCII"));

                String [] tokens = data.split("\n");

                if(tokens.length > 3) {
                    int nodeId = Integer.parseInt(tokens[2]);
                    byte pkType =(Byte.valueOf(tokens[1]));

                    if(node.getAddress().equals(nodeId) && packet.getType().code == pkType) {
                        int len = Integer.valueOf(tokens[0]);
                        VYPayload payload = null;

                        try {
                            payload = (VYPayload) packet.getType().payloadClass.newInstance();
                        }catch (Exception e){
                            e.printStackTrace();
                        }

                        payload.parse(tokens);

                        packet.setPayload(payload);

                        return packet;
                    }
                }
            }
        }
        return null;
    }
}
