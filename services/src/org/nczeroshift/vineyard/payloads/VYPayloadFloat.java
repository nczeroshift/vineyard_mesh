package org.nczeroshift.vineyard.payloads;

import org.nczeroshift.vineyard.VYPayload;
import org.nczeroshift.vineyard.VYUtils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class VYPayloadFloat extends VYPayload {
    private byte channel = 0;
    private float value = 0;

    public VYPayloadFloat(){

    }

    public byte getChannel() {
        return channel;
    }

    public void setChannel(byte channel) {
        this.channel = channel;
    }

    public float getValue() {
        return value;
    }

    public void setValue(float value) {
        this.value = value;
    }

    @Override
    public String toString() {
        return "VYPayloadFloat{" +
                "channel=" + channel +
                ", value=" + value +
                '}';
    }

    @Override
    public void parse(String payload[]) {
        if(payload.length>3) {
            channel = Byte.valueOf(payload[3].substring(0, 2));
            byte[] dataBytes = VYUtils.hexStringToByteArray(payload[3].substring(2));
            value = ByteBuffer.wrap(dataBytes).order(ByteOrder.LITTLE_ENDIAN).getFloat();
        }
    }

    @Override
    public String stream() {
        return null;
    }
}
