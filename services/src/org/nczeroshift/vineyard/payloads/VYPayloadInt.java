package org.nczeroshift.vineyard.payloads;

import org.nczeroshift.vineyard.VYPayload;
import org.nczeroshift.vineyard.VYUtils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class VYPayloadInt extends VYPayload {
    private byte channel = 0;
    private int value = 0;

    public VYPayloadInt(){

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

    public void setValue(int value) {
        this.value = value;
    }

    @Override
    public String toString() {
        return "VYPayloadInt{" +
                "channel=" + channel +
                ", value=" + value +
                '}';
    }

    @Override
    public void parse(String payload[]) {
        if(payload.length>1) {
            channel = Byte.valueOf(payload[0].substring(0, 2));
            byte[] dataBytes = VYUtils.hexStringToByteArray(payload[1].substring(2));
            value = ByteBuffer.wrap(dataBytes).order(ByteOrder.LITTLE_ENDIAN).getInt();
        }
    }

    @Override
    public String stream() {
        return null;
    }


}
