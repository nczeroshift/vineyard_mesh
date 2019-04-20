package org.nczeroshift.vineyard.payloads;

import org.nczeroshift.vineyard.VYPayload;
import org.nczeroshift.vineyard.VYUtils;

public class VYPayloadIO extends VYPayload {

    private byte channel = 0;
    private byte value = 0;

    public VYPayloadIO(){

    }

    public byte getChannel() {
        return channel;
    }

    public void setChannel(byte channel) {
        this.channel = channel;
    }

    public int getValue() {
        return value;
    }

    public void setValue(byte value) {
        this.value = value;
    }

    @Override
    public String toString() {
        return "VYPayloadIO{" +
                "channel=" + channel +
                ", value=" + value +
                '}';
    }

    @Override
    public void parse(String payload[]) {
        if(payload.length>1) {
            channel = Byte.valueOf(payload[0].substring(0, 2));
            byte[] dataBytes = VYUtils.hexStringToByteArray(payload[1].substring(2));
            value = dataBytes[0];
        }
    }

    @Override
    public String stream() {
        return null;
    }


}
