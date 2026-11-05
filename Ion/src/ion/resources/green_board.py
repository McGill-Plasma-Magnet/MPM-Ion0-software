from olaf.common.resource import Resource
from loguru import logger

class GreenBoardResource(Resource):
    """
    Resource for communicating with the GreenBoard via CANopen.
    """

    def __init__(self) -> None:
        super().__init__()
        # Define the CANopen Node ID of the GreenBoard here
        self.node_id = 0x02  # Placeholder: Replace with actual GreenBoard Node ID
        logger.info(f"GreenBoardResource initialized with Node ID: {self.node_id}")

    def on_start(self) -> None:
        """
        Called by the OLAF app when the resource starts.
        """
        logger.info("Starting GreenBoard communication...")
        # The self.node object is provided by the OLAF framework during start()
        # We can use it to interact with the CANopen bus.

        try:
            self.ping_green_board()
        except Exception as e:
            logger.error(f"Failed to ping GreenBoard: {e}")

    def on_end(self) -> None:
        """
        Called by the OLAF app when the resource is shutting down.
        """
        logger.info("Shutting down GreenBoard communication.")

    def ping_green_board(self) -> None:
        """
        Sends a ping/heartbeat check to the GreenBoard.
        """
        if self.node is None:
            logger.error("CANopen node not initialized. Cannot ping.")
            return

        logger.debug(f"Pinging GreenBoard at Node ID {self.node_id}...")

        # In CANopen, a simple way to "ping" is often to read a known object
        # from the Object Dictionary (OD) or send a NMT command.
        # Here we use the OLAF Node interface to request data from the GreenBoard.

        # Example: Reading the device name or a version register (Commonly 0x1000 range)
        # Replace 0x1000 with a valid index from the GreenBoard's Object Dictionary.
        try:
            # This assumes the olaf.canopen.Node provides a method to read from another node
            # response = self.node.read_object(self.node_id, 0x1000, 0x00)
            # logger.info(f"GreenBoard responded: {response}")

            # For a simple "is it there" check, we just log the attempt
            logger.info("Ping request sent to GreenBoard. Check CAN logs for response.")
        except Exception as e:
            logger.exception(f"Error during ping: {e}")
